"""Unreal ejecuta este archivo al abrir el editor (plugin Python Editor Script).

Si el kit de assets de The Unfinished Cut todavía no está importado, lo importa automáticamente
cuando el editor termina de cargar. Las siguientes aperturas no hacen nada.
"""
import importlib.util
from pathlib import Path

import unreal

_MARKER = "/Game/TUC/Kit/Meshes/SM_Watcher"
# Subir este número obliga a reimportar al abrir (p. ej. tras un git pull con assets nuevos).
IMPORT_VERSION = "6"
_handle = None


def _imported_version():
    marker = Path(unreal.Paths.project_dir()) / "Saved" / "TUC_ImportVersion.txt"
    try:
        return marker.read_text(encoding="utf-8").strip()
    except Exception:
        return ""


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
    if unreal.EditorAssetLibrary.does_asset_exist(_MARKER) and _imported_version() == IMPORT_VERSION:
        return
    unreal.log("TUC: importando/actualizando los assets del juego (2-4 minutos)...")
    try:
        _run_import()
    except Exception as error:  # nunca bloquear el editor
        unreal.log_error(f"TUC: la importación automática falló: {error}. Ejecuta Tools/import_external_assets.py a mano.")


_handle = unreal.register_slate_post_tick_callback(_on_tick)
