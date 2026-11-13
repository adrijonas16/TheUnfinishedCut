"""Optimiza un FBX de Meshy conservando el original.

Uso con Blender:
  blender --background --python Tools/optimize_meshy_asset.py -- input.fbx output.fbx 25000

v1.1: además de reducir caras, deja la malla lista para Unreal:
- sin referencias a imágenes (los FBX de Meshy traían rutas absolutas de otra máquina);
  las texturas se asignan en Unreal con Tools/import_external_assets.py;
- nombre de objeto y de malla = nombre del archivo (Interchange nombra por la malla);
- pivote en la base centrada.
"""

import sys
from pathlib import Path

import bpy
from mathutils import Vector


def args_after_separator():
    if "--" not in sys.argv:
        raise SystemExit("Faltan argumentos: input.fbx output.fbx target_faces")
    return sys.argv[sys.argv.index("--") + 1 :]


source_arg, destination_arg, target_arg = args_after_separator()
source = Path(source_arg).resolve()
destination = Path(destination_arg).resolve()
target_faces = int(target_arg)

bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath=str(source))

meshes = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
meshes.sort(key=lambda obj: len(obj.data.polygons), reverse=True)
if not meshes:
    raise SystemExit("El FBX no contiene mallas.")

hero = meshes[0]
original_faces = len(hero.data.polygons)

for obj in list(bpy.context.scene.objects):
    obj.select_set(False)
    if obj is not hero:
        bpy.data.objects.remove(obj, do_unlink=True)

hero.select_set(True)
bpy.context.view_layer.objects.active = hero
hero.name = destination.stem
hero.data.name = destination.stem

if original_faces > target_faces:
    modifier = hero.modifiers.new(name="TUC_GameReady_Decimate", type="DECIMATE")
    modifier.decimate_type = "COLLAPSE"
    modifier.ratio = max(0.01, min(1.0, target_faces / original_faces))
    modifier.use_collapse_triangulate = True
    bpy.ops.object.modifier_apply(modifier=modifier.name)

# Material único sin nodos de imagen.
clean_material = bpy.data.materials.new(f"M_{destination.stem.replace('SM_', '')}")
clean_material.use_nodes = True
hero.data.materials.clear()
hero.data.materials.append(clean_material)
for image in list(bpy.data.images):
    bpy.data.images.remove(image)

# Pivote en la base centrada.
bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
coords = [v.co.copy() for v in hero.data.vertices]
center_x = (min(c.x for c in coords) + max(c.x for c in coords)) / 2
center_y = (min(c.y for c in coords) + max(c.y for c in coords)) / 2
bottom_z = min(c.z for c in coords)
for vertex in hero.data.vertices:
    vertex.co -= Vector((center_x, center_y, bottom_z))

destination.parent.mkdir(parents=True, exist_ok=True)
bpy.ops.export_scene.fbx(
    filepath=str(destination),
    use_selection=True,
    object_types={"MESH"},
    apply_unit_scale=True,
    bake_space_transform=False,
    path_mode="STRIP",
    embed_textures=False,
    add_leaf_bones=False,
    mesh_smooth_type="FACE",
)

print(f"TUC_OPTIMIZED source_faces={original_faces} output_faces={len(hero.data.polygons)} output={destination}")
