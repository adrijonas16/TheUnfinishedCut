"""Importa en Unreal todos los assets del juego (se ejecuta solo al abrir el editor la primera vez).

También se puede lanzar a mano: Tools -> Execute Python Script -> Tools/import_external_assets.py
Es idempotente: vuelve a importar y reemplaza.

1. Kit TUC (ExternalAssets/TUC_Kit): 46 mallas low-poly, 17 texturas, material maestro + instancias.
   -> /Game/TUC/Kit/{Meshes,Textures,Materials}
2. Modelos Meshy (proyector y gato de referencia) con materiales PBR.
   -> /Game/TUC/Props/Projector, /Game/TUC/Characters/Nilo

El C++ busca las mallas por esas rutas. Si falta alguna, usa un respaldo y el juego sigue funcionando.
"""

import json
from pathlib import Path

import unreal


PROJECT_DIR = Path(unreal.Paths.project_dir())
ASSET_TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
EAL = unreal.EditorAssetLibrary
MEL = unreal.MaterialEditingLibrary

KIT_DIR = PROJECT_DIR / "ExternalAssets" / "TUC_Kit"
KIT_ROOT = "/Game/TUC/Kit"
SURF_DIR = KIT_DIR / "Surfaces"

# Debe coincidir con IMPORT_VERSION de Content/Python/init_unreal.py.
IMPORT_VERSION = "6"

# UE 5.5+ importa FBX con Interchange, que ignora las opciones FbxImportUI (escala, esqueleto,
# animaciones, materiales). Forzar el importador FBX clásico para que todo salga igual en 5.1–5.7.
for _cvar in ("Interchange.FeatureFlags.Import.FBX 0", "Interchange.FeatureFlags.Import.FBX.ToLevel 0"):
    try:
        unreal.SystemLibrary.execute_console_command(None, _cvar)
    except Exception:
        pass

# Los mapas normales de Meshy usan convención OpenGL (+Y). Unreal usa DirectX (-Y).
FLIP_NORMAL_GREEN = True

MESHY_ASSETS = [
    {
        "fbx": "ExternalAssets/Meshy/Optimized/Nilo_Base/SM_Nilo_Base_Optimized.fbx",
        "dest": "/Game/TUC/Characters/Nilo",
        "mesh": "SM_Nilo_Base_Optimized",
        "material": "M_Nilo_Base_PBR",
        "tex_dir": "ExternalAssets/Meshy/Optimized/Nilo_Base",
        "tex_prefix": "Meshy_AI_Blue_Eyed_Siamese_Cat_20260630_texture",
    },
    {
        "fbx": "ExternalAssets/Meshy/Optimized/Retro_Projector/SM_RetroProjector_Optimized.fbx",
        "dest": "/Game/TUC/Props/Projector",
        "mesh": "SM_RetroProjector_Optimized",
        "material": "M_RetroProjector_PBR",
        "tex_dir": "ExternalAssets/Meshy/Optimized/Retro_Projector",
        "tex_prefix": "Meshy_AI_Retro_Film_Projector_20260524_texture",
    },
]


REPORT = []


def log(message):
    unreal.log(f"TUC: {message}")
    REPORT.append(message)


def problem(message):
    unreal.log_warning(f"TUC: {message}")
    REPORT.append("PROBLEMA: " + message)


def engine_texture(path="/Engine/EngineResources/WhiteSquareTexture"):
    try:
        return unreal.load_asset(path)
    except Exception:
        return None


# ───────────────────────── utilidades ─────────────────────────
def run_import(filename, destination, options=None, name=None):
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", str(filename))
    task.set_editor_property("destination_path", destination)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)
    if name:
        task.set_editor_property("destination_name", name)
    if options is not None:
        task.set_editor_property("options", options)
    ASSET_TOOLS.import_asset_tasks([task])
    return list(task.get_editor_property("imported_object_paths") or [])


def fbx_static_mesh_options():
    ui = unreal.FbxImportUI()
    ui.set_editor_property("import_mesh", True)
    ui.set_editor_property("import_as_skeletal", False)
    ui.set_editor_property("import_animations", False)
    ui.set_editor_property("import_materials", False)
    ui.set_editor_property("import_textures", False)
    ui.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_STATIC_MESH)
    mesh_data = ui.get_editor_property("static_mesh_import_data")
    mesh_data.set_editor_property("combine_meshes", True)
    mesh_data.set_editor_property("auto_generate_collision", False)
    return ui


def find_static_mesh(destination, expected_name, known_before):
    expected_path = f"{destination}/{expected_name}"
    if EAL.does_asset_exist(expected_path):
        asset = EAL.load_asset(expected_path)
        if isinstance(asset, unreal.StaticMesh):
            return asset

    # Interchange (UE 5.5+) puede nombrar la malla de otra forma: tomar la nueva y renombrarla.
    for path in EAL.list_assets(destination, recursive=False, include_folder=False):
        current = path.split(".")[0]
        if current in known_before:
            continue
        asset = EAL.load_asset(path)
        if isinstance(asset, unreal.StaticMesh):
            log(f"renombrando {current} -> {expected_path}")
            EAL.rename_asset(current, expected_path)
            return EAL.load_asset(expected_path)
    return None


def import_mesh(fbx, destination, name):
    before = {p.split(".")[0] for p in EAL.list_assets(destination, recursive=False, include_folder=False)} if EAL.does_directory_exist(destination) else set()
    before.discard(f"{destination}/{name}")
    run_import(fbx, destination, fbx_static_mesh_options(), name)
    return find_static_mesh(destination, name, before)


def import_texture(source, destination, kind="color"):
    if not source.exists():
        problem(f"falta la textura {source}")
        return None
    paths = run_import(source, destination)
    texture = EAL.load_asset(paths[0]) if paths else EAL.load_asset(f"{destination}/{source.stem}")
    if not isinstance(texture, unreal.Texture2D):
        problem(f"no se pudo importar {source.name}")
        return None
    if kind == "normal":
        texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_NORMALMAP)
        texture.set_editor_property("srgb", False)
        texture.set_editor_property("flip_green_channel", FLIP_NORMAL_GREEN)
    elif kind == "mask":
        texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_MASKS)
        texture.set_editor_property("srgb", False)
    EAL.save_loaded_asset(texture)
    return texture


def assign_by_slot(mesh, materials, expected=None):
    """Asigna instancias de material por nombre de slot; si no coincide, por el orden del manifiesto."""
    slots = mesh.get_editor_property("static_materials")
    expected = expected or []
    for index, slot in enumerate(slots):
        slot_name = str(slot.get_editor_property("material_slot_name"))
        key = slot_name.split(".")[0]
        if key not in materials and "_" in key and key.rsplit("_", 1)[-1].isdigit():
            key = key.rsplit("_", 1)[0]
        material = materials.get(key)
        if material is None and index < len(expected):
            material = materials.get(expected[index])
        if material is None and len(expected) == 1:
            material = materials.get(expected[0])
        if material:
            mesh.set_material(index, material)
        else:
            problem(f"{mesh.get_name()} slot '{slot_name}' sin material")


# ───────────────────────── kit TUC ─────────────────────────
def build_master_material(white_texture):
    path = f"{KIT_ROOT}/Materials/M_TUC_Master"
    if EAL.does_asset_exist(path):
        # Se reconstruye siempre: una versión rota de una importación anterior dejaba todo en cuadros grises.
        EAL.delete_asset(path)
    white_texture = white_texture or engine_texture()

    material = ASSET_TOOLS.create_asset("M_TUC_Master", f"{KIT_ROOT}/Materials", unreal.Material, unreal.MaterialFactoryNew())

    tex = MEL.create_material_expression(material, unreal.MaterialExpressionTextureSampleParameter2D, -900, -100)
    tex.set_editor_property("parameter_name", "BaseTex")
    tex.set_editor_property("texture", white_texture)

    tint = MEL.create_material_expression(material, unreal.MaterialExpressionVectorParameter, -900, 200)
    tint.set_editor_property("parameter_name", "Tint")
    tint.set_editor_property("default_value", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))

    base = MEL.create_material_expression(material, unreal.MaterialExpressionMultiply, -550, 0)
    MEL.connect_material_expressions(tex, "RGB", base, "A")
    MEL.connect_material_expressions(tint, "", base, "B")
    MEL.connect_material_property(base, "", unreal.MaterialProperty.MP_BASE_COLOR)

    emissive = MEL.create_material_expression(material, unreal.MaterialExpressionScalarParameter, -550, 250)
    emissive.set_editor_property("parameter_name", "Emissive")
    emissive.set_editor_property("default_value", 0.0)
    glow = MEL.create_material_expression(material, unreal.MaterialExpressionMultiply, -300, 200)
    MEL.connect_material_expressions(base, "", glow, "A")
    MEL.connect_material_expressions(emissive, "", glow, "B")
    MEL.connect_material_property(glow, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)

    for name, default, prop, y in (("Roughness", 0.6, unreal.MaterialProperty.MP_ROUGHNESS, 450),
                                   ("Metallic", 0.0, unreal.MaterialProperty.MP_METALLIC, 550)):
        scalar = MEL.create_material_expression(material, unreal.MaterialExpressionScalarParameter, -550, y)
        scalar.set_editor_property("parameter_name", name)
        scalar.set_editor_property("default_value", default)
        MEL.connect_material_property(scalar, "", prop)

    MEL.recompile_material(material)
    EAL.save_loaded_asset(material)
    return material


def build_surface_master(defaults):
    """Material PBR para pisos y paredes: color * tinte, normal, rugosidad, AO (texturas ambientCG CC0)."""
    path = f"{KIT_ROOT}/Materials/M_TUC_Surface"
    if EAL.does_asset_exist(path):
        EAL.delete_asset(path)
    material = ASSET_TOOLS.create_asset("M_TUC_Surface", f"{KIT_ROOT}/Materials", unreal.Material, unreal.MaterialFactoryNew())

    def param(name, texture, sampler, x, y):
        node = MEL.create_material_expression(material, unreal.MaterialExpressionTextureSampleParameter2D, x, y)
        node.set_editor_property("parameter_name", name)
        node.set_editor_property("texture", texture)
        node.set_editor_property("sampler_type", sampler)
        return node

    base = param("BaseTex", defaults["BaseColor"], unreal.MaterialSamplerType.SAMPLERTYPE_COLOR, -900, -300)
    tint = MEL.create_material_expression(material, unreal.MaterialExpressionVectorParameter, -900, -100)
    tint.set_editor_property("parameter_name", "Tint")
    tint.set_editor_property("default_value", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))
    color = MEL.create_material_expression(material, unreal.MaterialExpressionMultiply, -550, -250)
    MEL.connect_material_expressions(base, "RGB", color, "A")
    MEL.connect_material_expressions(tint, "", color, "B")
    MEL.connect_material_property(color, "", unreal.MaterialProperty.MP_BASE_COLOR)

    normal = param("NormalTex", defaults["Normal"], unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL, -900, 100)
    MEL.connect_material_property(normal, "RGB", unreal.MaterialProperty.MP_NORMAL)

    rough = param("RoughTex", defaults["Roughness"], unreal.MaterialSamplerType.SAMPLERTYPE_MASKS, -900, 400)
    rough_mul = MEL.create_material_expression(material, unreal.MaterialExpressionScalarParameter, -900, 650)
    rough_mul.set_editor_property("parameter_name", "RoughnessMul")
    rough_mul.set_editor_property("default_value", 1.0)
    rough_out = MEL.create_material_expression(material, unreal.MaterialExpressionMultiply, -550, 450)
    MEL.connect_material_expressions(rough, "R", rough_out, "A")
    MEL.connect_material_expressions(rough_mul, "", rough_out, "B")
    MEL.connect_material_property(rough_out, "", unreal.MaterialProperty.MP_ROUGHNESS)

    metallic = MEL.create_material_expression(material, unreal.MaterialExpressionScalarParameter, -550, 750)
    metallic.set_editor_property("parameter_name", "Metallic")
    metallic.set_editor_property("default_value", 0.0)
    MEL.connect_material_property(metallic, "", unreal.MaterialProperty.MP_METALLIC)

    ao = param("AOTex", defaults.get("AO") or defaults["Roughness"], unreal.MaterialSamplerType.SAMPLERTYPE_MASKS, -900, 900)
    MEL.connect_material_property(ao, "R", unreal.MaterialProperty.MP_AMBIENT_OCCLUSION)

    MEL.recompile_material(material)
    EAL.save_loaded_asset(material)
    return material


def import_surfaces():
    """Devuelve {M_nombre: (spec, {BaseColor: Texture2D, ...})} con las texturas PBR reales de pisos y paredes."""
    listing = SURF_DIR / "surfaces.json"
    if not listing.exists():
        return {}
    kinds = {"BaseColor": "color", "Normal": "normal", "Roughness": "mask", "AO": "mask"}
    result = {}
    for name, spec in json.loads(listing.read_text(encoding="utf-8")).items():
        textures = {}
        for label, filename in spec.get("textures", {}).items():
            texture = import_texture(SURF_DIR / filename, f"{KIT_ROOT}/Surfaces", kinds.get(label, "color"))
            if texture:
                textures[label] = texture
        if "BaseColor" in textures and "Normal" in textures and "Roughness" in textures:
            result[name] = (spec, textures)
        else:
            problem(f"superficie {name} incompleta, se usa la textura del kit")
    log(f"{len(result)} superficies PBR (pisos/paredes) importadas")
    return result


def build_material_instance(name, spec, master, textures, surfaces=None, surface_master=None):
    path = f"{KIT_ROOT}/Materials/MI_{name}"
    if EAL.does_asset_exist(path):
        instance = EAL.load_asset(path)
    else:
        instance = ASSET_TOOLS.create_asset(f"MI_{name}", f"{KIT_ROOT}/Materials", unreal.MaterialInstanceConstant,
                                            unreal.MaterialInstanceConstantFactoryNew())
    if surfaces and surface_master and name in surfaces:
        surface_spec, surface_tex = surfaces[name]
        MEL.set_material_instance_parent(instance, surface_master)
        r, g, b = surface_spec.get("tint", [1.0, 1.0, 1.0])
        MEL.set_material_instance_vector_parameter_value(instance, "Tint", unreal.LinearColor(r, g, b, 1.0))
        MEL.set_material_instance_scalar_parameter_value(instance, "RoughnessMul", float(surface_spec.get("roughness_mul", 1.0)))
        MEL.set_material_instance_scalar_parameter_value(instance, "Metallic", float(surface_spec.get("metallic", 0.0)))
        MEL.set_material_instance_texture_parameter_value(instance, "BaseTex", surface_tex["BaseColor"])
        MEL.set_material_instance_texture_parameter_value(instance, "NormalTex", surface_tex["Normal"])
        MEL.set_material_instance_texture_parameter_value(instance, "RoughTex", surface_tex["Roughness"])
        MEL.set_material_instance_texture_parameter_value(instance, "AOTex", surface_tex.get("AO", surface_tex["Roughness"]))
        EAL.save_loaded_asset(instance)
        return instance

    MEL.set_material_instance_parent(instance, master)
    r, g, b = spec["color"]
    MEL.set_material_instance_vector_parameter_value(instance, "Tint", unreal.LinearColor(r, g, b, 1.0))
    MEL.set_material_instance_scalar_parameter_value(instance, "Emissive", float(spec["emissive"]))
    MEL.set_material_instance_scalar_parameter_value(instance, "Roughness", float(spec["roughness"]))
    MEL.set_material_instance_scalar_parameter_value(instance, "Metallic", float(spec["metallic"]))
    texture = textures.get(spec["texture"]) if spec["texture"] else textures.get("T_White")
    if texture:
        MEL.set_material_instance_texture_parameter_value(instance, "BaseTex", texture)
    EAL.save_loaded_asset(instance)
    return instance


def import_kit():
    manifest_path = KIT_DIR / "kit_manifest.json"
    if not manifest_path.exists():
        problem(f"no se encontró {manifest_path}")
        return False
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))

    log("importando texturas del kit...")
    textures = {}
    for png in sorted((KIT_DIR / "Textures").glob("*.png")):
        texture = import_texture(png, f"{KIT_ROOT}/Textures")
        if texture:
            textures[png.stem] = texture

    master = build_master_material(textures.get("T_White"))
    surfaces = import_surfaces()
    surface_master = build_surface_master(next(iter(surfaces.values()))[1]) if surfaces else None
    materials = {name: build_material_instance(name, spec, master, textures, surfaces, surface_master)
                 for name, spec in manifest["materials"].items()}
    log(f"{len(materials)} instancias de material listas")

    ok = 0
    names = sorted(manifest["meshes"].keys())
    with unreal.ScopedSlowTask(len(names), "TUC: importando kit de assets") as task:
        task.make_dialog(True)
        for name in names:
            if task.should_cancel():
                break
            task.enter_progress_frame(1, name)
            kit_fbx = KIT_DIR / "Meshes" / f"{name}.fbx"
            if not kit_fbx.exists():
                ok += 1  # solo existe como reemplazo Meshy (se importa después)
                continue
            mesh = import_mesh(kit_fbx, f"{KIT_ROOT}/Meshes", name)
            if mesh is None:
                problem(f"no se pudo importar {name}")
                continue
            assign_by_slot(mesh, materials, manifest["meshes"][name].get("materials"))
            EAL.save_loaded_asset(mesh)
            ok += 1
    log(f"kit: {ok}/{len(names)} mallas importadas")
    return ok == len(names)


# ───────────────────────── modelos Meshy ─────────────────────────
def add_texture_node(material, texture, x, y, sampler_type):
    node = MEL.create_material_expression(material, unreal.MaterialExpressionTextureSample, x, y)
    node.set_editor_property("texture", texture)
    node.set_editor_property("sampler_type", sampler_type)
    return node


def build_pbr_material(destination, name, textures):
    path = f"{destination}/{name}"
    if EAL.does_asset_exist(path):
        EAL.delete_asset(path)
    material = ASSET_TOOLS.create_asset(name, destination, unreal.Material, unreal.MaterialFactoryNew())
    if textures.get("base"):
        node = add_texture_node(material, textures["base"], -700, -300, unreal.MaterialSamplerType.SAMPLERTYPE_COLOR)
        MEL.connect_material_property(node, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
    if textures.get("normal"):
        node = add_texture_node(material, textures["normal"], -700, 300, unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL)
        MEL.connect_material_property(node, "RGB", unreal.MaterialProperty.MP_NORMAL)
    if textures.get("roughness"):
        node = add_texture_node(material, textures["roughness"], -700, 100, unreal.MaterialSamplerType.SAMPLERTYPE_MASKS)
        MEL.connect_material_property(node, "R", unreal.MaterialProperty.MP_ROUGHNESS)
    if textures.get("metallic"):
        node = add_texture_node(material, textures["metallic"], -700, -100, unreal.MaterialSamplerType.SAMPLERTYPE_MASKS)
        MEL.connect_material_property(node, "R", unreal.MaterialProperty.MP_METALLIC)
    if textures.get("emissive"):
        node = add_texture_node(material, textures["emissive"], -700, 500, unreal.MaterialSamplerType.SAMPLERTYPE_COLOR)
        boost = MEL.create_material_expression(material, unreal.MaterialExpressionMultiply, -350, 500)
        MEL.connect_material_expressions(node, "RGB", boost, "A")
        boost.set_editor_property("const_b", 6.0)
        MEL.connect_material_property(boost, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    MEL.recompile_material(material)
    EAL.save_loaded_asset(material)
    return material


def import_meshy(spec):
    fbx = PROJECT_DIR / spec["fbx"]
    if not fbx.exists():
        problem(f"no se encontró {fbx}")
        return False
    log(f"importando {fbx.name}")
    mesh = import_mesh(fbx, spec["dest"], spec["mesh"])
    if mesh is None:
        problem(f"la importación de {fbx.name} no produjo un Static Mesh")
        return False
    tex_dir = PROJECT_DIR / spec["tex_dir"]
    tex_dest = f"{spec['dest']}/Textures"
    prefix = spec["tex_prefix"]
    textures = {
        "base": import_texture(tex_dir / f"{prefix}.png", tex_dest),
        "normal": import_texture(tex_dir / f"{prefix}_normal.png", tex_dest, "normal"),
        "roughness": import_texture(tex_dir / f"{prefix}_roughness.png", tex_dest, "mask"),
        "metallic": import_texture(tex_dir / f"{prefix}_metallic.png", tex_dest, "mask"),
    }
    material = build_pbr_material(spec["dest"], spec["material"], textures)
    mesh.set_material(0, material)
    EAL.save_loaded_asset(mesh)
    return True


# ───────────────────────── Ivo animado (Meshy, esqueleto UE5 Mannequin) ─────────────────────────
IVO_DIR = PROJECT_DIR / "ExternalAssets" / "Meshy" / "Ivo_Animated"
IVO_DEST = "/Game/TUC/Characters/Ivo"


def fbx_skeletal_options():
    ui = unreal.FbxImportUI()
    ui.set_editor_property("import_mesh", True)
    ui.set_editor_property("import_as_skeletal", True)
    ui.set_editor_property("import_animations", True)
    ui.set_editor_property("import_materials", False)
    ui.set_editor_property("import_textures", False)
    ui.set_editor_property("create_physics_asset", False)
    ui.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_SKELETAL_MESH)
    return ui


def import_ivo():
    fbx_files = sorted(IVO_DIR.rglob("*.fbx")) if IVO_DIR.exists() else []
    if not fbx_files:
        log("Ivo animado no encontrado; se usará el modelo del kit.")
        return True
    fbx = fbx_files[0]
    log(f"importando Ivo animado: {fbx.name}")
    run_import(fbx, IVO_DEST, fbx_skeletal_options(), "SK_Ivo")

    skeletal = None
    for path in EAL.list_assets(IVO_DEST, recursive=False, include_folder=False):
        current = path.split(".")[0]
        asset = EAL.load_asset(path)
        short = current.rsplit("/", 1)[-1]
        if isinstance(asset, unreal.SkeletalMesh):
            if current != f"{IVO_DEST}/SK_Ivo":
                EAL.rename_asset(current, f"{IVO_DEST}/SK_Ivo")
            skeletal = EAL.load_asset(f"{IVO_DEST}/SK_Ivo")
        elif isinstance(asset, unreal.AnimSequence):
            lower = short.lower()
            target = "A_Ivo_Run" if "run" in lower else ("A_Ivo_Walk" if "walk" in lower else None)
            if target and short != target and not EAL.does_asset_exist(f"{IVO_DEST}/{target}"):
                EAL.rename_asset(current, f"{IVO_DEST}/{target}")

    if skeletal is None:
        unreal.log_error("TUC: la importación de Ivo no produjo un Skeletal Mesh")
        return False

    stem = fbx.stem
    folder = fbx.parent
    textures = {
        "base": import_texture(folder / f"{stem}.png", f"{IVO_DEST}/Textures"),
        "normal": import_texture(folder / f"{stem}_normal.png", f"{IVO_DEST}/Textures", "normal"),
        "roughness": import_texture(folder / f"{stem}_roughness.png", f"{IVO_DEST}/Textures", "mask"),
        "metallic": import_texture(folder / f"{stem}_metallic.png", f"{IVO_DEST}/Textures", "mask"),
    }
    material = build_pbr_material(IVO_DEST, "M_Ivo_PBR", textures)
    slots = skeletal.get_editor_property("materials")
    for slot in slots:
        slot.set_editor_property("material_interface", material)
    skeletal.set_editor_property("materials", slots)
    EAL.save_loaded_asset(skeletal)
    found = [EAL.does_asset_exist(f"{IVO_DEST}/{n}") for n in ("A_Ivo_Walk", "A_Ivo_Run")]
    log(f"Ivo animado listo (caminar: {found[0]}, correr: {found[1]})")
    return True


# ───────────────────────── reemplazos realistas (comunidad Meshy, CC0) ─────────────────────────
OVR_DIR = KIT_DIR / "Overrides"


def import_overrides():
    listing = OVR_DIR / "overrides.json"
    if not listing.exists():
        return True
    overrides = json.loads(listing.read_text(encoding="utf-8"))
    ok = True
    for name, info in sorted(overrides.items()):
        fbx = OVR_DIR / f"{name}.fbx"
        if not fbx.exists():
            continue
        log(f"reemplazo realista: {name} ({info.get('source')})")
        mesh = import_mesh(fbx, f"{KIT_ROOT}/Meshes", name)
        if mesh is None:
            problem(f"no se pudo importar el reemplazo {name}")
            ok = False
            continue
        tex = info.get("textures", {})
        kinds = {"BaseColor": ("base", "color"), "Normal": ("normal", "normal"), "Roughness": ("roughness", "mask"),
                 "Metallic": ("metallic", "mask"), "Emissive": ("emissive", "color")}
        textures = {}
        for label, filename in tex.items():
            key, kind = kinds.get(label, (None, None))
            if key:
                textures[key] = import_texture(OVR_DIR / filename, f"{KIT_ROOT}/Overrides/Textures", kind)
        material = build_pbr_material(f"{KIT_ROOT}/Overrides", f"M_{name}", textures)
        for index in range(len(mesh.get_editor_property("static_materials"))):
            mesh.set_material(index, material)
        EAL.save_loaded_asset(mesh)
    return ok


# ───────────────────────── Nilo animado (gato esmoquin, Meshy CC0) ─────────────────────────
NILO_FBX = PROJECT_DIR / "ExternalAssets" / "Meshy" / "Community" / "Cat"
NILO_DEST = "/Game/TUC/Characters/Nilo"


def import_nilo():
    files = sorted(NILO_FBX.glob("*.fbx")) if NILO_FBX.exists() else []
    if not files:
        return True
    log(f"importando Nilo animado: {files[0].name}")
    # Borrar la importación anterior (podía quedar sin textura o sin animación enlazada).
    for old in ("SK_Nilo", "A_Nilo_Walk", "SK_Nilo_Skeleton", "SK_Nilo_PhysicsAsset", "M_Nilo_PBR"):
        if EAL.does_asset_exist(f"{NILO_DEST}/{old}"):
            EAL.delete_asset(f"{NILO_DEST}/{old}")
    ui = fbx_skeletal_options()
    # El FBX de Meshy está en una escala diminuta; el C++ lo ajusta a 70 cm de todos modos.
    ui.get_editor_property("skeletal_mesh_import_data").set_editor_property("import_uniform_scale", 100.0)
    ui.get_editor_property("anim_sequence_import_data").set_editor_property("import_uniform_scale", 100.0)
    before = {p.split(".")[0] for p in EAL.list_assets(NILO_DEST, recursive=False, include_folder=False)} if EAL.does_directory_exist(NILO_DEST) else set()
    run_import(files[0], NILO_DEST, ui, "SK_Nilo")

    skeletal = None
    anims = []
    for path in EAL.list_assets(NILO_DEST, recursive=True, include_folder=False):
        current = path.split(".")[0]
        if current in before and current != f"{NILO_DEST}/SK_Nilo":
            continue
        asset = EAL.load_asset(path)
        if isinstance(asset, unreal.SkeletalMesh):
            if current != f"{NILO_DEST}/SK_Nilo":
                EAL.rename_asset(current, f"{NILO_DEST}/SK_Nilo")
            skeletal = EAL.load_asset(f"{NILO_DEST}/SK_Nilo")
        elif isinstance(asset, unreal.AnimSequence):
            anims.append(current)
    if anims and not EAL.does_asset_exist(f"{NILO_DEST}/A_Nilo_Walk"):
        # La toma más larga es el ciclo de caminar.
        def length(path):
            try:
                return float(EAL.load_asset(path).get_play_length())
            except Exception:
                return 0.0
        anims.sort(key=length, reverse=True)
        EAL.rename_asset(anims[0], f"{NILO_DEST}/A_Nilo_Walk")

    # Textura del gato: viene incrustada en el FBX; la extrajimos a T_Nilo_BaseColor.png.
    if skeletal is not None:
        base = import_texture(NILO_FBX / "T_Nilo_BaseColor.png", f"{NILO_DEST}/Textures")
        if base:
            material = build_pbr_material(NILO_DEST, "M_Nilo_PBR", {"base": base})
            slots = skeletal.get_editor_property("materials")
            for slot in slots:
                slot.set_editor_property("material_interface", material)
            skeletal.set_editor_property("materials", slots)
            EAL.save_loaded_asset(skeletal)

    ready = EAL.does_asset_exist(f"{NILO_DEST}/SK_Nilo") and EAL.does_asset_exist(f"{NILO_DEST}/A_Nilo_Walk")
    log(f"Nilo animado listo: {ready} (animaciones encontradas: {len(anims)})")
    return ready


# ───────────────────────── interfaz (menú, cómo jugar, botones) ─────────────────────────
UI_DIR = PROJECT_DIR / "ExternalAssets" / "TUC_UI"
UI_DEST = "/Game/TUC/UI"


def import_ui():
    files = sorted(UI_DIR.glob("T_UI_*.png")) if UI_DIR.exists() else []
    ok = 0
    for png in files:
        paths = run_import(png, UI_DEST)
        texture = EAL.load_asset(paths[0]) if paths else EAL.load_asset(f"{UI_DEST}/{png.stem}")
        if not isinstance(texture, unreal.Texture2D):
            problem(f"no se pudo importar la imagen de interfaz {png.name}")
            continue
        texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_EDITOR_ICON)
        texture.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
        # El atlas de la fuente se dibuja reducido: con mipmaps se ve suave; el resto, sin mipmaps (nítido).
        mips = unreal.TextureMipGenSettings.TMGS_SIMPLE_AVERAGE if png.stem == "T_UI_FontAtlas" else unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS
        texture.set_editor_property("mip_gen_settings", mips)
        texture.set_editor_property("never_stream", True)
        texture.set_editor_property("srgb", True)
        EAL.save_loaded_asset(texture)
        ok += 1
    log(f"interfaz: {ok}/{len(files)} imágenes")
    return ok == len(files)


# ───────────────────────── ejecución ─────────────────────────
results = [import_ui(), import_kit(), import_overrides()] + [import_meshy(spec) for spec in MESHY_ASSETS] + [import_ivo(), import_nilo()]
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)

try:
    (PROJECT_DIR / "Saved").mkdir(parents=True, exist_ok=True)
    (PROJECT_DIR / "Saved" / "TUC_ImportReport.txt").write_text("\n".join(REPORT), encoding="utf-8")
except Exception:
    pass

try:
    marker = PROJECT_DIR / "Saved" / "TUC_ImportVersion.txt"
    marker.parent.mkdir(parents=True, exist_ok=True)
    marker.write_text(IMPORT_VERSION, encoding="utf-8")
except Exception as error:
    problem(f"no se pudo guardar la versión de importación: {error}")

problems = [line for line in REPORT if line.startswith("PROBLEMA")]
if problems:
    unreal.EditorDialog.show_message("The Unfinished Cut — importación",
        f"Hubo {len(problems)} problema(s). Detalle en Saved/TUC_ImportReport.txt:\n\n" + "\n".join(problems[:12]),
        unreal.AppMsgType.OK)
elif all(results):
    log("importación terminada. Presiona Play.")
    unreal.EditorDialog.show_message("The Unfinished Cut", "Assets importados. Ya puedes presionar Play.", unreal.AppMsgType.OK)
if not all(results):
    unreal.log_error("TUC: algunos assets no se importaron. Revisa Window > Output Log (filtra por 'TUC').")
