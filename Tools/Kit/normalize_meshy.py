"""Convierte modelos descargados de la comunidad de Meshy (CC0) en reemplazos del kit TUC.

Para cada entrada de OVERRIDES:
  - reduce polígonos, gira para que el frente mire a +X, escala a la medida objetivo y apoya la base en Z=0;
  - copia las texturas a 2048 px con nombres fijos (<SM>_BaseColor/_Normal/_Roughness/_Metallic/_Emissive.png);
  - escribe ExternalAssets/TUC_Kit/Overrides/<SM>.fbx y actualiza bounds en kit_manifest.json
    (layout.py recalcula las colisiones con esas medidas).

Uso: python3 normalize_meshy.py <carpeta Community>
"""
import glob
import json
import math
import os
import sys

import bpy
from mathutils import Matrix, Vector
from PIL import Image

PROJECT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
KIT = os.path.join(PROJECT, "ExternalAssets", "TUC_Kit")
OUT = os.path.join(KIT, "Overrides")
SRC = sys.argv[-1]

# asset: (carpeta, giro Z en grados, modo de ajuste, medida en m, caras objetivo, copias en fila)
OVERRIDES = {
    "SM_ArcadeCabinet":  ("Arcade",   90, "height", 1.95, 20000, 1),
    "SM_Barrier":        ("Barrier",  90, "length", 1.8, 12000, 1),
    "SM_Hydrant":        ("Hydrant",  90, "height", 0.75, 12000, 1),
    "SM_Jukebox":        ("Jukebox",  90, "height", 1.6, 20000, 1),
    "SM_StreetLamp":     ("Lamp",    180, "height", 4.4, 15000, 1),
    "SM_PowerConsole":   ("Panel",    90, "height", 1.0, 25000, 1),
    "SM_PopcornCart":    ("Popcorn",  90, "height", 1.7, 25000, 1),
    "SM_FilmReelStand":  ("Reel",     90, "height", 0.55, 15000, 1),
    "SM_SeatRow":        ("Seat",     90, "height", 1.0, 8000, 5),
    "SM_Watcher":        ("Sentinel", 90, "height", 1.1, 30000, 1),
    "SM_ReelShelf":      ("Shelf",    90, "height", 2.4, 10000, 1),
    "SM_Speaker":        ("Speaker",  90, "height", 1.15, 12000, 1),
    "SM_Terminal":       ("Terminal", 90, "height", 1.35, 20000, 1),
    "SM_ProjectionScreen": ("Curtain", 90, "length", 9.0, 25000, 1),
    "SM_ConcessionCounter": ("Booth", 90, "height", 2.1, 25000, 1),
    "SM_Van":            ("Van",      90, "length", 4.4, 25000, 1),
    "SM_FutureCrate":    ("Crate",    90, "height", 0.85, 15000, 1),
    "SM_TrashBin":       ("Trash",    90, "height", 0.95, 12000, 1),
}
CENTER_PIVOT = {"SM_Watcher"}
# pivote en el poste (extremo -X) en lugar del centro
POLE_PIVOT = {"SM_StreetLamp"}
MAPS = {"": "BaseColor", "_normal": "Normal", "_roughness": "Roughness", "_metallic": "Metallic", "_emission": "Emissive"}


def build(name, folder, rot, mode, size, faces, copies):
    bpy.ops.wm.read_factory_settings(use_empty=True)
    fbx = glob.glob(os.path.join(SRC, folder, "*.fbx"))[0]
    bpy.ops.import_scene.fbx(filepath=fbx)
    meshes = [o for o in bpy.context.scene.objects if o.type == "MESH"]
    for o in list(bpy.context.scene.objects):
        if o.type != "MESH":
            bpy.data.objects.remove(o, do_unlink=True)
    bpy.ops.object.select_all(action="DESELECT")
    for o in meshes:
        o.select_set(True)
    bpy.context.view_layer.objects.active = meshes[0]
    if len(meshes) > 1:
        bpy.ops.object.join()
    obj = bpy.context.view_layer.objects.active
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)

    current = len(obj.data.polygons)
    if current > faces:
        mod = obj.modifiers.new("decimate", "DECIMATE")
        mod.ratio = faces / current
        bpy.ops.object.modifier_apply(modifier=mod.name)

    me = obj.data
    me.transform(Matrix.Rotation(math.radians(rot), 4, "Z"))
    co = [v.co for v in me.vertices]
    mn = Vector([min(c[i] for c in co) for i in range(3)])
    mx = Vector([max(c[i] for c in co) for i in range(3)])
    dims = mx - mn
    ref = dims.z if mode == "height" else max(dims.x, dims.y)
    s = size / ref
    center = (mn + mx) / 2
    pivot_z = center.z if name in CENTER_PIVOT else mn.z
    pivot_x = (mn.x + 0.08 / s) if name in POLE_PIVOT else center.x
    me.transform(Matrix.Diagonal((s, s, s, 1)) @ Matrix.Translation(-Vector((pivot_x, center.y, pivot_z))))

    if copies > 1:
        width = dims.y * s
        bpy.ops.object.select_all(action="DESELECT")
        obj.select_set(True)
        parts = [obj]
        for i in range(1, copies):
            dup = obj.copy()
            dup.data = obj.data.copy()
            bpy.context.scene.collection.objects.link(dup)
            parts.append(dup)
        for i, p in enumerate(parts):
            p.data.transform(Matrix.Translation((0, (i - (copies - 1) / 2) * width, 0)))
            p.select_set(True)
        bpy.context.view_layer.objects.active = obj
        bpy.ops.object.join()
        obj = bpy.context.view_layer.objects.active

    obj.name = name
    obj.data.name = name
    mat = bpy.data.materials.new(f"M_Ovr_{name}")
    obj.data.materials.clear()
    obj.data.materials.append(mat)
    for img in list(bpy.data.images):
        bpy.data.images.remove(img)

    os.makedirs(OUT, exist_ok=True)
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.ops.export_scene.fbx(filepath=os.path.join(OUT, name + ".fbx"), use_selection=True, object_types={"MESH"},
                             apply_unit_scale=True, bake_space_transform=False, path_mode="STRIP",
                             embed_textures=False, add_leaf_bones=False, mesh_smooth_type="FACE")

    # texturas
    stem = os.path.splitext(os.path.basename(fbx))[0]
    textures = {}
    for suffix, label in MAPS.items():
        src = os.path.join(SRC, folder, f"{stem}{suffix}.png")
        if os.path.exists(src):
            im = Image.open(src)
            if max(im.size) > 2048:
                im = im.resize((2048, 2048), Image.LANCZOS)
            dst = f"{name}_{label}.png"
            im.save(os.path.join(OUT, dst), optimize=True)
            textures[label] = dst

    vs = [v.co for v in obj.data.vertices]
    bmn = [min(c[i] for c in vs) for i in range(3)]
    bmx = [max(c[i] for c in vs) for i in range(3)]
    return {
        "faces": len(obj.data.polygons),
        "size_m": [round(b - a, 3) for a, b in zip(bmn, bmx)],
        # bounds en espacio Unreal (cm, Y invertida)
        "bounds_cm": [[round(bmn[0] * 100, 1), round(-bmx[1] * 100, 1), round(bmn[2] * 100, 1)],
                      [round(bmx[0] * 100, 1), round(-bmn[1] * 100, 1), round(bmx[2] * 100, 1)]],
        "textures": textures,
        "source": folder,
    }


def main():
    manifest_path = os.path.join(KIT, "kit_manifest.json")
    manifest = json.load(open(manifest_path))
    report = {}
    only = os.environ.get("TUC_ONLY")
    if only:
        report = json.load(open(os.path.join(OUT, "overrides.json")))
    for name, spec in OVERRIDES.items():
        if only and name not in only.split(","):
            continue
        folder = spec[0]
        if not glob.glob(os.path.join(SRC, folder, "*.fbx")):
            print("SKIP", name, "(sin FBX)")
            continue
        info = build(name, *spec)
        report[name] = info
        entry = manifest["meshes"].setdefault(name, {"materials": []})
        entry["size_m"] = info["size_m"]
        entry["bounds_cm"] = info["bounds_cm"]
        entry["override"] = True
        print("OVERRIDE", name, info["faces"], info["size_m"])
    json.dump(report, open(os.path.join(OUT, "overrides.json"), "w"), indent=2)
    json.dump(manifest, open(manifest_path, "w"), indent=2)


main()
