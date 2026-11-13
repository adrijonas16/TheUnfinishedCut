"""Unreal ejecuta este archivo al abrir el editor (plugin Python Editor Script).

Si el kit de assets de The Unfinished Cut todavía no está importado, lo importa automáticamente
cuando el editor termina de cargar. Las siguientes aperturas no hacen nada.
"""
import importlib.util
from pathlib import Path

import unreal

_MARKER = "/Game/TUC/Kit/Meshes/SM_Watcher"
_handle = None


def _run_import():
    script = Path(unreal.Paths.project_dir()) / "Tools" / "import_external_assets.py"
    if not script.exists():
        unreal.log_error(f"TUC: no se encontró {script}")
        return
    spec = importlib.util.spec_from_file_location("tuc_import_external_assets", str(script))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)


def _on_tick(_delta):
    global _handle
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    if registry.is_loading_assets():
        return
    unreal.unregister_slate_post_tick_callback(_handle)
    _handle = None
    if unreal.EditorAssetLibrary.does_asset_exist(_MARKER):
        return
    unreal.log("TUC: primera apertura del proyecto, importando el kit de assets (1-2 minutos)...")
    try:
        _run_import()
    except Exception as error:  # nunca bloquear el editor
        unreal.log_error(f"TUC: la importación automática falló: {error}. Ejecuta Tools/import_external_assets.py a mano.")


_handle = unreal.register_slate_post_tick_callback(_on_tick)
