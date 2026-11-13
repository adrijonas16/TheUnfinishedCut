"""Crea un mapa vacío propio para el proyecto (opcional).

El juego funciona en cualquier mapa porque el GameMode genera todo en runtime.
Un mapa propio sirve para dejar de depender de /Engine/Maps/Entry y para añadir
iluminación o decorado a mano más adelante.

Uso: Tools -> Execute Python Script -> Tools/create_prototype_map.py
Después: Project Settings -> Maps & Modes -> Editor Startup Map y Game Default Map = L0_Campaign.
"""

import unreal

MAP_PATH = "/Game/TUC/Maps/L0_Campaign"

if unreal.EditorAssetLibrary.does_asset_exist(MAP_PATH):
    unreal.log(f"TUC: {MAP_PATH} ya existe; no se sobrescribe.")
else:
    unreal.log("TUC: creando mapa vacío de la campaña...")
    subsystem = None
    if hasattr(unreal, "LevelEditorSubsystem"):
        subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if subsystem is not None:
        subsystem.new_level(MAP_PATH)
    else:
        unreal.EditorLevelLibrary.new_level(MAP_PATH)

    # Forzar el GameMode del proyecto en este mapa, aunque cambie el GameMode global.
    world = unreal.EditorLevelLibrary.get_editor_world()
    if world:
        settings = world.get_world_settings()
        game_mode = unreal.load_class(None, "/Script/TheUnfinishedCut.TUCGameMode")
        if settings and game_mode:
            settings.set_editor_property("default_game_mode", game_mode)

    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
    unreal.log("TUC: mapa creado. Asígnalo en Project Settings -> Maps & Modes.")
