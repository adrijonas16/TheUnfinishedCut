"""Reconstruye el nivel completo en Blender a partir de layout.json y renderiza vistas de verificación."""
import json, math, os, sys
import bpy
from mathutils import Vector

PROJECT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
ROOT = os.path.join(PROJECT, "ExternalAssets", "TUC_Kit")
OUTDIR = sys.argv[-2]
VIEWS = sys.argv[-1].split(",")
os.makedirs(OUTDIR, exist_ok=True)
L = json.load(open(os.path.join(ROOT, "layout.json")))
MAN = json.load(open(os.path.join(ROOT, "kit_manifest.json")))
PROJ = PROJECT + "/ExternalAssets/Meshy/Optimized/Retro_Projector/SM_RetroProjector_Optimized.fbx"
PROJ_TEX = PROJECT + "/ExternalAssets/Meshy/Optimized/Retro_Projector/Meshy_AI_Retro_Film_Projector_20260524_texture.png"

bpy.ops.wm.read_factory_settings(use_empty=True)
sc = bpy.context.scene
cache = {}


def ue(x, y, z):
    return Vector((x / 100, -y / 100, z / 100))


def load(asset):
    if asset in cache:
        return cache[asset]
    ovr = os.path.join(ROOT, "Overrides", asset + ".fbx")
    path = PROJ if asset == "PROJECTOR" else (ovr if os.path.exists(ovr) else os.path.join(ROOT, "Meshes", asset + ".fbx"))
    bpy.ops.import_scene.fbx(filepath=path)
    o = [o for o in bpy.context.selected_objects if o.type == "MESH"][0]
    if asset == "PROJECTOR":
        s = 1.1 / max(o.dimensions.x, o.dimensions.y)
        o.data.transform(__import__("mathutils").Matrix.Diagonal((s, s, s, 1)))
        m = o.data.materials[0]; m.use_nodes = True
        t = m.node_tree.nodes.new("ShaderNodeTexImage"); t.image = bpy.data.images.load(PROJ_TEX)
        m.node_tree.links.new(t.outputs["Color"], m.node_tree.nodes["Principled BSDF"].inputs["Base Color"])
    o.hide_render = True; o.hide_viewport = True
    cache[asset] = o
    return o


def place(asset, x, y, z, yaw, scale=(1, 1, 1), tint=None):
    src = load(asset)
    o = src.copy()
    o.hide_render = False; o.hide_viewport = False
    sc.collection.objects.link(o)
    o.location = ue(x, y, z)
    o.rotation_euler = (0, 0, math.radians(-yaw))
    o.scale = scale
    return o


for a, x, y, z, yaw, sx, sy, sz in L["visuals"]:
    place(a, x, y, z, yaw, (sx, sy, sz))

TINTS = [(0.1, 0.8, 1.0), (0.2, 1.0, 0.3), (0.6, 0.15, 1.0), (0.2, 1.0, 0.3), (0.6, 0.15, 1.0), (1, 0.8, 0.15),
         (0.1, 0.8, 1.0), (0.2, 1.0, 0.3), (0.6, 0.15, 1.0), (1, 0.8, 0.15), (1, 0.8, 0.15), (1, 1, 1),
         (0.1, 0.8, 1.0), (1, 0.8, 0.15), (1, 0.55, 0.05)]
for (a, x, y, yaw), tint in zip(L["interactables"], TINTS):
    z = 18 if a == "SM_AnchorFragment" else (26 if x == 2740 else -5)
    place(a, x, y, z, yaw)
# personajes en el checkpoint 1 y en el Archivo
place("SM_Ivo", 300, -250, -5, 0)
place("SM_Nilo", 260, -150, -5, 0)
place("SM_Ivo", 4580, 0, -5, 0)
place("SM_Nilo", 4520, 110, -5, 30)
for wx in (1250, 2400, 3650, 5150):
    place("SM_Watcher", wx, 120, 110, 90)

# materiales a partir del manifiesto
for m in bpy.data.materials:
    base = m.name.split(".")[0]
    if base.startswith("M_Ovr_"):
        tex = os.path.join(ROOT, "Overrides", base[6:] + "_BaseColor.png")
        m.use_nodes = True
        nt = m.node_tree; b = nt.nodes.get("Principled BSDF")
        if os.path.exists(tex) and b:
            t = nt.nodes.new("ShaderNodeTexImage"); t.image = bpy.data.images.load(tex)
            nt.links.new(t.outputs["Color"], b.inputs["Base Color"])
            emi = tex.replace("_BaseColor", "_Emissive")
            if os.path.exists(emi):
                e = nt.nodes.new("ShaderNodeTexImage"); e.image = bpy.data.images.load(emi)
                nt.links.new(e.outputs["Color"], b.inputs["Emission Color"]); b.inputs["Emission Strength"].default_value = 3
        continue
    spec = MAN["materials"].get(base)
    if not spec:
        continue
    m.use_nodes = True
    nt = m.node_tree; b = nt.nodes.get("Principled BSDF")
    col = spec["color"]
    if base == "M_AccentGlow":
        col = (1.0, 0.75, 0.25)
    b.inputs["Base Color"].default_value = (*col, 1)
    b.inputs["Roughness"].default_value = spec["roughness"]
    b.inputs["Metallic"].default_value = spec["metallic"]
    if spec["texture"]:
        t = nt.nodes.new("ShaderNodeTexImage"); t.image = bpy.data.images.load(os.path.join(ROOT, "Textures", spec["texture"] + ".png"))
        nt.links.new(t.outputs["Color"], b.inputs["Base Color"])
        if spec["emissive"] > 0:
            nt.links.new(t.outputs["Color"], b.inputs["Emission Color"])
    elif spec["emissive"] > 0:
        b.inputs["Emission Color"].default_value = (*col, 1)
    b.inputs["Emission Strength"].default_value = spec["emissive"] * 0.6

for x, y, z, r, g, bl, inten, rad in L["lights"]:
    ld = bpy.data.lights.new("p", "POINT"); ld.color = (r, g, bl); ld.energy = inten * 0.12; ld.shadow_soft_size = 0.3
    lo = bpy.data.objects.new("p", ld); sc.collection.objects.link(lo); lo.location = ue(x, y, z)

# cielo de crepúsculo
w = bpy.data.worlds.new("w"); w.use_nodes = True
sky = w.node_tree.nodes.new("ShaderNodeTexSky")
try:
    sky.sky_type = "NISHITA"; sky.sun_elevation = math.radians(4); sky.sun_rotation = math.radians(200)
except Exception:
    pass
w.node_tree.links.new(sky.outputs["Color"], w.node_tree.nodes["Background"].inputs["Color"])
w.node_tree.nodes["Background"].inputs["Strength"].default_value = 0.35
sc.world = w

sc.render.engine = "CYCLES"; sc.cycles.samples = 40; sc.cycles.use_denoising = True
sc.render.resolution_x = 1100; sc.render.resolution_y = 620
sc.view_settings.view_transform = "AgX" if "AgX" in [i.identifier for i in sc.view_settings.bl_rna.properties["view_transform"].enum_items] else "Filmic"
sc.view_settings.exposure = 1.0

CAMS = {
    "street": ((-1150, -250, 230), (0, 0, 250)),
    "cinema": ((150, -380, 260), (1300, 250, 120)),
    "cinema2": ((1200, 300, 200), (300, -440, 60)),
    "retro": ((1720, -300, 230), (2800, 150, 80)),
    "future": ((3080, 380, 260), (4300, -150, 80)),
    "archive": ((4500, -380, 260), (5650, 50, 150)),
}
cam = bpy.data.cameras.new("c"); cam.lens = 22
co = bpy.data.objects.new("c", cam); sc.collection.objects.link(co); sc.camera = co
for v in VIEWS:
    eye, target = CAMS[v]
    co.location = ue(*eye)
    d = ue(*target) - co.location
    co.rotation_euler = d.to_track_quat("-Z", "Y").to_euler()
    sc.render.filepath = os.path.join(OUTDIR, f"level_{v}.png")
    bpy.ops.render.render(write_still=True)
    print("RENDERED", v)
