"""Render de arte conceptual para el menú: la calle del cine Eternal Wave de noche, con Ivo y Nilo.

Usa los mismos assets del juego (kit, reemplazos Meshy, texturas ambientCG, personajes Meshy).
Salida: imagen con fondo transparente (el cielo se pinta después en make_ui.py) y pase de niebla (mist).

    python3 Tools/UI/render_keyart.py -- <salida_dir> [muestras]
"""
import json
import math
import os
import sys

import bpy
from mathutils import Matrix, Vector

PROJECT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
KIT = os.path.join(PROJECT, "ExternalAssets", "TUC_Kit")
MESHY = os.path.join(PROJECT, "ExternalAssets", "Meshy")
argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
OUTDIR = argv[0] if argv else "/tmp/keyart"
SAMPLES = int(argv[1]) if len(argv) > 1 else 96
RES = (int(argv[2]), int(argv[3])) if len(argv) > 3 else (1920, 1080)
os.makedirs(OUTDIR, exist_ok=True)

L = json.load(open(os.path.join(KIT, "layout.json")))
MAN = json.load(open(os.path.join(KIT, "kit_manifest.json")))
SURF = json.load(open(os.path.join(KIT, "Surfaces", "surfaces.json")))

bpy.ops.wm.read_factory_settings(use_empty=True)
sc = bpy.context.scene
cache = {}


def ue(x, y, z):
    return Vector((x / 100, -y / 100, z / 100))


def load(asset):
    if asset in cache:
        return cache[asset]
    ovr = os.path.join(KIT, "Overrides", asset + ".fbx")
    path = ovr if os.path.exists(ovr) else os.path.join(KIT, "Meshes", asset + ".fbx")
    bpy.ops.import_scene.fbx(filepath=path)
    o = [o for o in bpy.context.selected_objects if o.type == "MESH"][0]
    o.hide_render = True
    o.hide_viewport = True
    cache[asset] = o
    return o


def place(asset, x, y, z, yaw, scale=(1, 1, 1)):
    o = load(asset).copy()
    o.hide_render = False
    o.hide_viewport = False
    sc.collection.objects.link(o)
    o.location = ue(x, y, z)
    o.rotation_euler = (0, 0, math.radians(-yaw))
    o.scale = scale
    return o


# ── escenario: calle y fachada del cine ──
for a, x, y, z, yaw, sx, sy, sz in L["visuals"]:
    if -1350 < x < 260:
        place(a, x, y, z, yaw, (sx, sy, sz))
# suelo extra hacia la cámara para que no se vea el borde
for gx in range(-1900, -1250, 100):
    for gy in range(-550, 600, 100):
        place("SM_Floor_Asphalt" if abs(gy) < 300 else "SM_Floor_Sidewalk", gx, gy, -5, 0)


def tex_node(nt, path, non_color=False):
    t = nt.nodes.new("ShaderNodeTexImage")
    t.image = bpy.data.images.load(path, check_existing=True)
    if non_color:
        t.image.colorspace_settings.name = "Non-Color"
    return t


for m in bpy.data.materials:
    base = m.name.split(".")[0]
    m.use_nodes = True
    nt = m.node_tree
    b = nt.nodes.get("Principled BSDF")
    if not b:
        continue
    if base.startswith("M_Ovr_"):
        stem = os.path.join(KIT, "Overrides", base[6:])
        if os.path.exists(stem + "_BaseColor.png"):
            nt.links.new(tex_node(nt, stem + "_BaseColor.png").outputs["Color"], b.inputs["Base Color"])
        if os.path.exists(stem + "_Roughness.png"):
            nt.links.new(tex_node(nt, stem + "_Roughness.png", True).outputs["Color"], b.inputs["Roughness"])
        if os.path.exists(stem + "_Emissive.png"):
            e = tex_node(nt, stem + "_Emissive.png")
            nt.links.new(e.outputs["Color"], b.inputs["Emission Color"])
            b.inputs["Emission Strength"].default_value = 4
        continue
    if base in SURF:
        spec = SURF[base]
        files = spec["textures"]
        folder = os.path.join(KIT, "Surfaces")
        col = tex_node(nt, os.path.join(folder, files["BaseColor"]))
        mix = nt.nodes.new("ShaderNodeMixRGB")
        mix.blend_type = "MULTIPLY"
        mix.inputs["Fac"].default_value = 1.0
        mix.inputs["Color2"].default_value = (*spec["tint"], 1)
        nt.links.new(col.outputs["Color"], mix.inputs["Color1"])
        nt.links.new(mix.outputs["Color"], b.inputs["Base Color"])
        rough = tex_node(nt, os.path.join(folder, files["Roughness"]), True)
        rmul = nt.nodes.new("ShaderNodeMath")
        rmul.operation = "MULTIPLY"
        # Asfalto mojado: reflejos de las luces del cine.
        rmul.inputs[1].default_value = 0.35 if base in ("M_Asphalt", "M_Sidewalk") else spec.get("roughness_mul", 1.0)
        nt.links.new(rough.outputs["Color"], rmul.inputs[0])
        nt.links.new(rmul.outputs[0], b.inputs["Roughness"])
        nrm = tex_node(nt, os.path.join(folder, files["Normal"]), True)
        nmap = nt.nodes.new("ShaderNodeNormalMap")
        nt.links.new(nrm.outputs["Color"], nmap.inputs["Color"])
        nt.links.new(nmap.outputs["Normal"], b.inputs["Normal"])
        b.inputs["Metallic"].default_value = spec.get("metallic", 0.0)
        continue
    spec = MAN["materials"].get(base)
    if not spec:
        continue
    col = spec["color"]
    if base == "M_AccentGlow":
        col = (1.0, 0.75, 0.25)
    b.inputs["Base Color"].default_value = (*col, 1)
    b.inputs["Roughness"].default_value = spec["roughness"]
    b.inputs["Metallic"].default_value = spec["metallic"]
    if spec["texture"]:
        t = tex_node(nt, os.path.join(KIT, "Textures", spec["texture"] + ".png"))
        nt.links.new(t.outputs["Color"], b.inputs["Base Color"])
        if spec["emissive"] > 0:
            nt.links.new(t.outputs["Color"], b.inputs["Emission Color"])
    elif spec["emissive"] > 0:
        b.inputs["Emission Color"].default_value = (*col, 1)
    b.inputs["Emission Strength"].default_value = spec["emissive"] * 0.9

# ── letrero del cine ──
font_path = os.path.join(PROJECT, "ExternalAssets", "TUC_UI", "Fonts", "Griffy-Regular.ttf")
for text, x, y, z, yaw, size, r, g, bl in L["signs"]:
    if x > 260:
        continue
    cu = bpy.data.curves.new("sign", "FONT")
    cu.body = text
    cu.align_x = "CENTER"
    cu.align_y = "CENTER"
    cu.size = size / 100 * 1.25
    cu.extrude = 0.01
    if os.path.exists(font_path) and size > 30:
        cu.font = bpy.data.fonts.load(font_path, check_existing=True)
    o = bpy.data.objects.new("sign", cu)
    sc.collection.objects.link(o)
    o.location = ue(x - 6, y, z)
    o.rotation_euler = (math.radians(90), 0, math.radians(-90))
    mat = bpy.data.materials.new("sign")
    mat.use_nodes = True
    pb = mat.node_tree.nodes["Principled BSDF"]
    pb.inputs["Base Color"].default_value = (r / 255, g / 255, bl / 255, 1)
    pb.inputs["Emission Color"].default_value = (r / 255, g / 255, bl / 255, 1)
    pb.inputs["Emission Strength"].default_value = 12 if size > 30 else 5
    cu.materials.append(mat)


# ── personajes ──
def import_character(fbx, tex, height=None, length=None, frame=10):
    before = set(bpy.data.objects)
    bpy.ops.import_scene.fbx(filepath=fbx)
    new = [o for o in bpy.data.objects if o not in before]
    arm = next((o for o in new if o.type == "ARMATURE"), None)
    acts = [a for a in bpy.data.actions if a.users == 0 or True]
    if arm and bpy.data.actions:
        act = sorted(bpy.data.actions, key=lambda a: ("walk" not in a.name.lower(), a.name))[0]
        arm.animation_data_create()
        arm.animation_data.action = act
    sc.frame_set(frame)
    img = bpy.data.images.load(tex)
    meshes = [o for o in new if o.type == "MESH"]
    for me in meshes:
        for m in me.data.materials:
            m.use_nodes = True
            nt = m.node_tree
            pb = next((n for n in nt.nodes if n.type == "BSDF_PRINCIPLED"), None) or nt.nodes.new("ShaderNodeBsdfPrincipled")
            t = nt.nodes.new("ShaderNodeTexImage")
            t.image = img
            nt.links.new(t.outputs["Color"], pb.inputs["Base Color"])
            pb.inputs["Roughness"].default_value = 0.75
    roots = [o for o in new if o.parent is None]
    dg = bpy.context.evaluated_depsgraph_get()
    pts = []
    for me in meshes:
        ev = me.evaluated_get(dg)
        mm = ev.to_mesh()
        pts += [ev.matrix_world @ v.co for v in mm.vertices]
    lo = Vector((min(p.x for p in pts), min(p.y for p in pts), min(p.z for p in pts)))
    hi = Vector((max(p.x for p in pts), max(p.y for p in pts), max(p.z for p in pts)))
    size = hi - lo
    s = (height / size.z) if height else (length / max(size.x, size.y))
    pivot = bpy.data.objects.new("pivot", None)
    sc.collection.objects.link(pivot)
    for r in roots:
        r.parent = pivot
        r.matrix_parent_inverse = Matrix.Translation(-Vector(((lo.x + hi.x) / 2, (lo.y + hi.y) / 2, lo.z)))
    pivot.scale = (s, s, s)
    return pivot


ivo_dir = os.path.join(MESHY, "Ivo_Animated")
ivo_fbx = next(os.path.join(r, f) for r, _, fs in os.walk(ivo_dir) for f in fs if f.endswith(".fbx"))
ivo = import_character(ivo_fbx, ivo_fbx[:-4] + ".png", height=1.80, frame=12)
cat_dir = os.path.join(MESHY, "Community", "Cat")
cat_fbx = next(os.path.join(cat_dir, f) for f in os.listdir(cat_dir) if f.endswith(".fbx"))
nilo = import_character(cat_fbx, os.path.join(cat_dir, "T_Nilo_BaseColor.png"), length=0.78, frame=8)

# Caminan hacia el cine (+X en Unreal = +X en Blender). Los modelos Meshy miran a -Y en Blender.
ivo.location = ue(-780, 70, -5)
ivo.rotation_euler = (0, 0, math.radians(90 + 25))
nilo.location = ue(-690, -45, -5)
nilo.rotation_euler = (0, 0, math.radians(90 - 35))

# ── luces ──
for x, y, z, r, g, bl, inten, rad in L["lights"]:
    if x > 260:
        continue
    ld = bpy.data.lights.new("p", "POINT")
    ld.color = (r, g, bl)
    ld.energy = inten * 0.22
    ld.shadow_soft_size = 0.4
    lo = bpy.data.objects.new("p", ld)
    sc.collection.objects.link(lo)
    lo.location = ue(x, y, z)


def area(name, loc, rot, size, energy, color):
    ld = bpy.data.lights.new(name, "AREA")
    ld.size = size
    ld.energy = energy
    ld.color = color
    o = bpy.data.objects.new(name, ld)
    sc.collection.objects.link(o)
    o.location = loc
    o.rotation_euler = rot
    return o


# luz cálida que sale de la puerta del cine
area("door", ue(-40, 0, 150), (0, math.radians(-90), 0), 2.5, 900, (1.0, 0.62, 0.3))
# luna: luz fría de contra
moon = bpy.data.lights.new("moon", "SUN")
moon.energy = 0.9
moon.color = (0.55, 0.6, 1.0)
moon.angle = math.radians(1.5)
mo = bpy.data.objects.new("moon", moon)
sc.collection.objects.link(mo)
mo.rotation_euler = (math.radians(60), 0, math.radians(-100))
# relleno violeta suave
area("fill", ue(-1500, -300, 400), (math.radians(60), 0, math.radians(-120)), 6, 400, (0.55, 0.35, 1.0))
# borde para separar a los personajes del fondo
area("rim", ue(-600, 250, 260), (math.radians(70), 0, math.radians(150)), 1.5, 260, (0.6, 1.0, 0.85))

world = bpy.data.worlds.new("w")
world.use_nodes = True
world.node_tree.nodes["Background"].inputs[0].default_value = (0.05, 0.03, 0.12, 1)
world.node_tree.nodes["Background"].inputs[1].default_value = 0.5
sc.world = world

# ── cámara ──
cam = bpy.data.cameras.new("cam")
cam.lens = 30
cam.dof.use_dof = True
cam.dof.aperture_fstop = 2.8
co = bpy.data.objects.new("cam", cam)
sc.collection.objects.link(co)
sc.camera = co
co.location = ue(*[float(v) for v in os.environ.get("KA_EYE","-1150,-250,230").split(",")])
target = ue(*[float(v) for v in os.environ.get("KA_TGT","0,0,250").split(",")])
co.rotation_euler = (target - co.location).to_track_quat("-Z", "Y").to_euler()
cam.dof.focus_distance = (ue(-740, 20, 100) - co.location).length

sc.render.engine = "CYCLES"
sc.cycles.device = "CPU"
sc.cycles.samples = SAMPLES
sc.cycles.use_denoising = True
sc.render.resolution_x, sc.render.resolution_y = RES
sc.render.film_transparent = True
sc.view_settings.view_transform = "AgX" if "AgX" in [i.identifier for i in sc.view_settings.bl_rna.properties["view_transform"].enum_items] else "Filmic"
sc.view_settings.look = "None"
sc.view_settings.exposure = 0.6
sc.render.filepath = os.path.join(OUTDIR, "keyart.png")
bpy.ops.render.render(write_still=True)

# Segunda pasada: profundidad (para la niebla) con un material de emisión sobre todo.
depth = bpy.data.materials.new("depth")
depth.use_nodes = True
nt = depth.node_tree
nt.nodes.clear()
camdata = nt.nodes.new("ShaderNodeCameraData")
remap = nt.nodes.new("ShaderNodeMapRange")
remap.inputs["From Min"].default_value = 3.0
remap.inputs["From Max"].default_value = 30.0
emit = nt.nodes.new("ShaderNodeEmission")
out = nt.nodes.new("ShaderNodeOutputMaterial")
nt.links.new(camdata.outputs["View Z Depth"], remap.inputs["Value"])
nt.links.new(remap.outputs["Result"], emit.inputs["Color"])
nt.links.new(emit.outputs["Emission"], out.inputs["Surface"])
sc.view_layers[0].material_override = depth
cam.dof.use_dof = False
sc.cycles.samples = 4
sc.cycles.use_denoising = False
sc.view_settings.view_transform = "Standard"
sc.view_settings.exposure = 0.0
sc.render.filepath = os.path.join(OUTDIR, "depth.png")
bpy.ops.render.render(write_still=True)
print("KEYART OK")
