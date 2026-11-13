"""Genera el kit de assets low-poly de The Unfinished Cut y lo exporta a FBX + manifiesto.

Uso: python3 make_assets.py <out_dir>   (con el módulo bpy instalado)
"""
import json
import math
import os
import sys

import bpy

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from kitlib import Kit, MATERIALS, export  # noqa: E402

OUT = sys.argv[-1]
MESH_DIR = os.path.join(OUT, "Meshes")
os.makedirs(MESH_DIR, exist_ok=True)
ASSETS = {}


def asset(fn):
    ASSETS[fn.__name__] = fn
    return fn


# ───────────────────────── superficies modulares ─────────────────────────
def floor_tile(name, mat, trim=None):
    k = Kit(name)
    k.box((0, 0, -0.05), (1.0, 1.0, 0.1), mat)
    return k


for _n, _m in [("SM_Floor_Asphalt", "M_Asphalt"), ("SM_Floor_Sidewalk", "M_Sidewalk"), ("SM_Floor_Carpet", "M_Carpet"),
               ("SM_Floor_Checker", "M_Checker"), ("SM_Floor_Metal", "M_MetalPanel"), ("SM_Floor_Stone", "M_StoneTile")]:
    ASSETS[_n] = (lambda n=_n, m=_m: floor_tile(n, m))


def wall(name, mat, extras):
    """Segmento de pared 1 m (Y) x 0.2 m (X) x 6 m; la cara interior mira a +X."""
    k = Kit(name)
    k.box((0, 0, 3.0), (0.2, 1.0, 6.0), mat)
    k.box((0.11, 0, 0.08), (0.04, 1.0, 0.16), "M_Black")  # zócalo
    for z, h, d, m in extras:
        k.box((0.1 + d / 2, 0, z), (d, 1.0, h), m)
    return k


WALLS = {
    "SM_Wall_Brick":   ("M_Brick", [(5.9, 0.2, 0.08, "M_Concrete")]),
    "SM_Wall_Cinema":  ("M_CinemaPanel", [(1.05, 0.06, 0.05, "M_Gold"), (4.2, 0.08, 0.05, "M_Gold"), (5.85, 0.3, 0.12, "M_FacadeRed")]),
    "SM_Wall_Retro":   ("M_RetroGrid", [(1.2, 0.05, 0.04, "M_NeonMagenta"), (3.2, 0.05, 0.04, "M_NeonCyan")]),
    "SM_Wall_Future":  ("M_MetalPanel", [(0.6, 0.05, 0.04, "M_NeonCyan"), (4.6, 0.08, 0.04, "M_NeonCyan"), (2.6, 0.5, 0.06, "M_DarkMetal")]),
    "SM_Wall_Archive": ("M_StoneBlock", [(2.5, 0.04, 0.03, "M_NeonAmber"), (5.8, 0.4, 0.1, "M_Black")]),
}
for _n, (_m, _e) in WALLS.items():
    ASSETS[_n] = (lambda n=_n, m=_m, e=_e: wall(n, m, e))


# ───────────────────────── calle ─────────────────────────
@asset
def SM_StreetLamp():
    k = Kit("SM_StreetLamp")
    k.cyl((0, 0, 0.15), 0.16, 0.3, "M_DarkMetal", segs=8)
    k.cyl((0, 0, 2.3), 0.06, 4.3, "M_DarkMetal", segs=8)
    k.box((0.45, 0, 4.4), (0.9, 0.08, 0.08), "M_DarkMetal")
    k.box((0.85, 0, 4.3), (0.45, 0.3, 0.16), "M_DarkMetal")
    k.box((0.85, 0, 4.21), (0.38, 0.24, 0.03), "M_LampWarm")
    return k


@asset
def SM_Van():
    k = Kit("SM_Van")
    k.box((-0.3, 0, 1.15), (3.4, 1.9, 1.7), "M_VanRed")          # caja
    k.box((1.75, 0, 0.9), (0.9, 1.85, 1.2), "M_VanRed")           # cabina baja
    k.box((1.55, 0, 1.75), (0.6, 1.8, 0.5), "M_VanRed", rot=(0, -25, 0))
    k.box((1.72, 0, 1.6), (0.05, 1.6, 0.45), "M_Glass", rot=(0, -25, 0))  # parabrisas
    k.box((-0.3, 0.96, 1.4), (2.6, 0.02, 0.5), "M_Glass")
    k.box((-0.3, -0.96, 1.4), (2.6, 0.02, 0.5), "M_Glass")
    k.box((2.21, 0.7, 0.75), (0.02, 0.3, 0.15), "M_LampWarm")
    k.box((2.21, -0.7, 0.75), (0.02, 0.3, 0.15), "M_LampWarm")
    k.box((-2.01, 0.75, 0.9), (0.02, 0.2, 0.3), "M_NeonRed")
    k.box((-2.01, -0.75, 0.9), (0.02, 0.2, 0.3), "M_NeonRed")
    k.box((2.25, 0, 0.4), (0.12, 1.9, 0.2), "M_Steel")
    for x in (1.4, -1.4):
        for y in (0.88, -0.88):
            k.cyl((x, y, 0.35), 0.35, 0.25, "M_Rubber", segs=12, rot=(90, 0, 0))
            k.cyl((x, y * 1.02, 0.35), 0.2, 0.26, "M_Steel", segs=8, rot=(90, 0, 0))
    return k


@asset
def SM_Barrier():
    k = Kit("SM_Barrier")
    for y in (-0.7, 0.7):
        k.box((0, y, 0.45), (0.08, 0.08, 0.9), "M_White")
        k.box((0, y, 0.03), (0.5, 0.12, 0.06), "M_DarkMetal")
    for i in range(8):
        y = -0.8 + 0.2 * i + 0.1
        k.box((0, y, 0.75), (0.06, 0.2, 0.22), "M_Orange" if i % 2 == 0 else "M_White")
        k.box((0, y, 0.4), (0.06, 0.2, 0.18), "M_White" if i % 2 == 0 else "M_Orange")
    k.cyl((0, 0.7, 0.98), 0.07, 0.1, "M_NeonAmber", segs=8)
    return k


def building(name, width, height, neon):
    k = Kit(name)
    k.box((-0.5, 0, height / 2), (1.0, width, height), "M_Concrete")
    k.box((0.01, 0, height / 2 + 0.6), (0.04, width - 0.4, height - 1.6), "M_Windows")
    k.box((0.05, 0, 0.5), (0.1, width, 1.0), "M_Brick")
    k.box((0.0, 0, height + 0.1), (1.1, width + 0.1, 0.2), "M_DarkMetal")
    if neon:
        k.box((0.12, width / 2 - 0.5, height * 0.6), (0.08, 0.25, height * 0.45), neon)
    k.box((0.08, -width / 4, 1.2), (0.06, 1.0, 2.2), "M_Black")  # puerta
    return k


ASSETS["SM_Building_A"] = lambda: building("SM_Building_A", 5.5, 10.0, "M_NeonCyan")
ASSETS["SM_Building_B"] = lambda: building("SM_Building_B", 6.0, 14.0, "M_NeonMagenta")


@asset
def SM_Hydrant():
    k = Kit("SM_Hydrant")
    k.cyl((0, 0, 0.3), 0.12, 0.6, "M_NeonRed" if False else "M_VanRed", segs=8)
    k.sphere((0, 0, 0.62), (0.12, 0.12, 0.1), "M_VanRed", segs=8, rings=4)
    k.cyl((0, 0, 0.42), 0.05, 0.4, "M_Steel", segs=6, rot=(90, 0, 0))
    return k


@asset
def SM_TrashBin():
    k = Kit("SM_TrashBin")
    k.cyl((0, 0, 0.45), 0.28, 0.9, "M_Teal", segs=10)
    k.cyl((0, 0, 0.93), 0.3, 0.06, "M_DarkMetal", segs=10)
    return k


# ───────────────────────── cine ─────────────────────────
@asset
def SM_CinemaFacadeSide():
    """Muro de fachada 4 m (Y) x 0.5 m x 6 m; la cara de calle es +X (se coloca con yaw 180)."""
    k = Kit("SM_CinemaFacadeSide")
    k.box((0, 0, 3.0), (0.5, 4.0, 6.0), "M_FacadeRed")
    for y in (-1.9, 1.9):
        k.box((0.3, y, 3.0), (0.12, 0.25, 6.0), "M_Gold")
    k.box((0.3, 0, 5.85), (0.15, 4.0, 0.3), "M_Gold")
    k.box((0.27, 0, 0.3), (0.06, 4.0, 0.6), "M_Black")
    # vitrina de póster con marco y foco
    k.box((0.28, 0, 1.8), (0.06, 1.2, 1.7), "M_Gold")
    k.box((0.32, 0, 1.8), (0.02, 1.0, 1.5), "M_Poster0")
    k.box((0.4, 0, 2.8), (0.25, 0.5, 0.08), "M_DarkMetal")
    k.box((0.4, 0, 2.75), (0.2, 0.4, 0.02), "M_LampWarm")
    return k


@asset
def SM_CinemaMarquee():
    """Dintel sobre la puerta (4 m) con marquesina hacia +X (calle) de 1.25 m."""
    k = Kit("SM_CinemaMarquee")
    k.box((0, 0, 4.5), (0.5, 4.0, 3.0), "M_FacadeRed")
    k.box((0.875, 0, 4.0), (1.25, 3.4, 0.8), "M_Black")      # cartel
    k.box((1.5, 0, 4.0), (0.02, 3.2, 0.62), "M_DarkMetal")   # frente del cartel (texto en C++)
    k.box((0.875, 0, 3.55), (1.3, 3.5, 0.1), "M_NeonRed")
    k.box((0.875, 0, 4.45), (1.3, 3.5, 0.1), "M_NeonRed")
    for i in range(12):
        y = -1.6 + i * (3.2 / 11)
        k.sphere((1.52, y, 4.38), (0.04, 0.04, 0.04), "M_LampWarm", segs=6, rings=4)
        k.sphere((1.52, y, 3.62), (0.04, 0.04, 0.04), "M_LampWarm", segs=6, rings=4)
    k.box((0.3, 0, 3.05), (0.12, 4.0, 0.1), "M_Gold")
    # torre del letrero vertical
    k.box((0.6, 0, 5.4), (0.5, 0.25, 1.2), "M_Black")
    k.box((0.86, 0, 5.4), (0.02, 0.18, 1.1), "M_NeonRed")
    return k


@asset
def SM_ConcessionCounter():
    """Mostrador 2.5 m (Y) x 0.8 m (X) x 1.05 m con máquina de palomitas. Frente +X."""
    k = Kit("SM_ConcessionCounter")
    k.box((0, 0, 0.5), (0.8, 2.5, 1.0), "M_Wood")
    k.box((0.41, 0, 0.5), (0.02, 2.3, 0.8), "M_RedVelvet")
    k.box((0, 0, 1.03), (0.86, 2.56, 0.06), "M_Gold")
    k.box((0.05, -0.6, 1.25), (0.5, 0.9, 0.4), "M_Glass")
    k.box((0.42, 0, 0.12), (0.04, 2.5, 0.06), "M_NeonAmber")
    # máquina de palomitas
    k.box((0, 0.7, 1.4), (0.5, 0.5, 0.7), "M_Glass")
    k.box((0, 0.7, 1.08), (0.55, 0.55, 0.06), "M_VanRed")
    k.box((0, 0.7, 1.8), (0.55, 0.55, 0.12), "M_VanRed")
    k.box((0, 0.7, 1.25), (0.42, 0.42, 0.2), "M_NeonAmber")
    k.cone((0, 0.7, 1.95), 0.2, 0.2, "M_VanRed", segs=8)
    return k


@asset
def SM_LobbyPartition():
    """Mampara de vestíbulo 2.6 m (Y) x 0.4 m x 1.6 m con pósters por ambas caras."""
    k = Kit("SM_LobbyPartition")
    k.box((0, 0, 0.8), (0.4, 2.6, 1.6), "M_Wood")
    k.box((0, 0, 1.62), (0.46, 2.66, 0.06), "M_Gold")
    for i, y in enumerate((-0.7, 0.7)):
        for side in (1, -1):
            k.box((side * 0.21, y, 0.95), (0.02, 0.9, 1.1), "M_Gold")
            k.box((side * 0.225, y, 0.95), (0.01, 0.8, 1.0), f"M_Poster{(i + (side > 0)) % 3}")
    return k


@asset
def SM_SeatRow():
    """Fila de 6 butacas, 3.3 m (Y) x 0.8 m (X); miran a +X."""
    k = Kit("SM_SeatRow")
    k.box((0, 0, 0.12), (0.8, 3.3, 0.24), "M_Black")
    for i in range(6):
        y = -1.375 + i * 0.55
        k.box((0.05, y, 0.42), (0.5, 0.48, 0.14), "M_RedVelvet")
        k.box((-0.25, y, 0.8), (0.12, 0.48, 0.75), "M_RedVelvet", rot=(0, -8, 0))
        k.box((-0.31, y, 0.8), (0.03, 0.44, 0.7), "M_Black", rot=(0, -8, 0))
    for i in range(7):
        y = -1.65 + i * 0.55
        k.box((0.0, y, 0.6), (0.55, 0.06, 0.08), "M_Black")
        k.box((0.0, y, 0.38), (0.4, 0.05, 0.3), "M_DarkMetal")
    return k


@asset
def SM_ProjectionScreen():
    """Pantalla 9 m (Y) con telón; mira a +X (se coloca con yaw 180)."""
    k = Kit("SM_ProjectionScreen")
    k.box((0, 0, 0.25), (0.6, 9.0, 0.5), "M_Wood")
    k.box((0.31, 0, 0.25), (0.02, 9.0, 0.4), "M_NeonAmber")
    k.box((-0.1, 0, 2.8), (0.12, 7.2, 3.9), "M_Black")
    k.box((-0.02, 0, 2.8), (0.02, 6.8, 3.6), "M_ScreenWhite")
    for side in (-1, 1):
        for i in range(6):
            y = side * (3.55 + i * 0.16)
            k.box((0.05 + (i % 2) * 0.06, y, 2.9), (0.08, 0.17, 5.6), "M_RedVelvet")
    k.box((0.1, 0, 5.4), (0.3, 9.0, 0.6), "M_RedVelvet")
    k.box((0.26, 0, 5.1), (0.03, 9.0, 0.06), "M_Gold")
    return k


# ───────────────────────── años 80 ─────────────────────────
@asset
def SM_ArcadeCabinet():
    k = Kit("SM_ArcadeCabinet")
    k.box((-0.05, 0, 0.9), (0.6, 0.7, 1.8), "M_Purple")
    k.box((0.27, 0, 1.0), (0.12, 0.66, 0.12), "M_Black")           # panel de mandos
    k.cyl((0.28, -0.12, 1.11), 0.012, 0.1, "M_Steel", segs=6)
    k.sphere((0.28, -0.12, 1.17), (0.03, 0.03, 0.03), "M_NeonRed", segs=6, rings=4)
    for j, y in enumerate((0.05, 0.13, 0.21)):
        k.cyl((0.28, y, 1.075), 0.025, 0.02, ["M_NeonCyan", "M_NeonMagenta", "M_NeonAmber"][j], segs=8)
    k.box((0.24, 0, 1.45), (0.02, 0.56, 0.5), "M_ArcadeScreen", rot=(0, -15, 0))
    k.box((0.2, 0, 1.92), (0.12, 0.7, 0.24), "M_NeonMagenta")      # marquesina
    for y in (0.36, -0.36):
        k.box((0.25, y, 0.9), (0.03, 0.025, 1.8), "M_NeonCyan")
        k.box((-0.05, y, 1.79), (0.62, 0.025, 0.03), "M_NeonCyan")
    return k


@asset
def SM_Stage():
    """Tarima 3 m (X) x 3.5 m (Y) x 0.3 m con borde de neón."""
    k = Kit("SM_Stage")
    k.box((0, 0, 0.15), (3.0, 3.5, 0.3), "M_Black")
    k.box((0, 0, 0.305), (2.9, 3.4, 0.01), "M_Checker")
    k.box((1.51, 0, 0.15), (0.02, 3.5, 0.05), "M_NeonMagenta")
    k.box((0, 1.76, 0.15), (3.0, 0.02, 0.05), "M_NeonCyan")
    k.box((0, -1.76, 0.15), (3.0, 0.02, 0.05), "M_NeonCyan")
    return k


@asset
def SM_Speaker():
    k = Kit("SM_Speaker")
    k.box((0, 0, 0.6), (0.6, 0.6, 1.2), "M_Black")
    for z, r in ((0.35, 0.2), (0.85, 0.12), (1.07, 0.06)):
        k.cyl((0.31, 0, z), r, 0.03, "M_DarkMetal", segs=12, rot=(0, 90, 0))
        k.cyl((0.325, 0, z), r * 0.4, 0.02, "M_Steel", segs=8, rot=(0, 90, 0))
    return k


@asset
def SM_Jukebox():
    k = Kit("SM_Jukebox")
    k.box((0, 0, 0.6), (0.6, 1.0, 1.2), "M_Wood")
    k.cyl((0, 0, 1.2), 0.5, 0.6, "M_Wood", segs=16, rot=(0, 90, 0))
    k.cyl((0.02, 0, 1.2), 0.42, 0.6, "M_NeonMagenta", segs=16, rot=(0, 90, 0))
    k.box((0.31, 0, 0.75), (0.02, 0.7, 0.5), "M_Glass")
    k.box((0.32, 0, 0.35), (0.02, 0.7, 0.04), "M_NeonCyan")
    return k


# ───────────────────────── futuro ─────────────────────────
@asset
def SM_SciFiTower():
    """Columna 1 x 1 x 3.4 m con bandas de luz."""
    k = Kit("SM_SciFiTower")
    k.cyl((0, 0, 1.7), 0.5, 3.4, "M_DarkMetal", segs=8)
    for z in (0.5, 1.6, 2.7):
        k.cyl((0, 0, z), 0.52, 0.06, "M_NeonCyan", segs=8)
    k.cyl((0, 0, 3.45), 0.3, 0.1, "M_Steel", segs=8)
    k.cyl((0, 0, 0.05), 0.56, 0.1, "M_Steel", segs=8)
    return k


@asset
def SM_DuctFrame():
    """Marco de ducto de pared 1.8 m (Y) x 0.25 m x 1.1 m; frente +X."""
    k = Kit("SM_DuctFrame")
    k.box((0, 0, 0.55), (0.25, 1.8, 1.1), "M_DarkMetal")
    k.box((0.13, 0, 0.55), (0.02, 1.5, 0.8), "M_Black")
    for i in range(5):
        k.box((0.14, 0, 0.25 + i * 0.15), (0.02, 1.45, 0.04), "M_Steel")
    k.box((0.13, 0, 1.03), (0.03, 1.8, 0.05), "M_NeonGreen")
    return k


@asset
def SM_FutureCrate():
    k = Kit("SM_FutureCrate")
    k.box((0, 0, 0.4), (0.8, 0.8, 0.8), "M_Teal")
    k.box((0, 0, 0.4), (0.84, 0.6, 0.6), "M_DarkMetal")
    k.box((0.43, 0, 0.6), (0.02, 0.3, 0.05), "M_NeonCyan")
    return k


@asset
def SM_HoloPanel():
    """Pantalla de pared 1.6 x 0.9 m (decorativa), frente +X."""
    k = Kit("SM_HoloPanel")
    k.box((0, 0, 0.45), (0.06, 1.6, 0.9), "M_DarkMetal")
    k.box((0.035, 0, 0.45), (0.01, 1.5, 0.8), "M_ArcadeScreen")
    return k


# ───────────────────────── archivo ─────────────────────────
@asset
def SM_Monolith():
    k = Kit("SM_Monolith")
    k.box((0, 0, 1.3), (0.35, 0.35, 2.6), "M_StoneBlock")
    for s in (1, -1):
        k.box((s * 0.18, 0, 1.4), (0.01, 0.2, 2.0), "M_FilmStrip")
    k.box((0, 0, 2.63), (0.4, 0.4, 0.06), "M_NeonAmber")
    return k


@asset
def SM_ReelShelf():
    """Estantería de latas de película 2 m (Y) x 0.5 m x 2.4 m; frente +X."""
    k = Kit("SM_ReelShelf")
    for y in (-0.98, 0.98):
        k.box((0, y, 1.2), (0.5, 0.04, 2.4), "M_Wood")
    k.box((-0.24, 0, 1.2), (0.02, 2.0, 2.4), "M_Black")
    for i in range(5):
        z = 0.05 + i * 0.58
        k.box((0, 0, z), (0.5, 2.0, 0.04), "M_Wood")
        if i < 4:
            for j in range(6):
                k.cyl((0.02, -0.75 + j * 0.3, z + 0.17), 0.14, 0.05, "M_Steel" if (i + j) % 3 else "M_Gold", segs=10, rot=(0, 90, 0))
    k.box((0.26, 0, 2.42), (0.02, 2.0, 0.03), "M_NeonAmber")
    return k


@asset
def SM_PrologueDoor():
    """Puerta del prólogo: marco 1.3 m x 2.3 m, hoja entreabierta y luz ámbar; frente +X."""
    k = Kit("SM_PrologueDoor")
    k.box((0, -0.6, 1.1), (0.3, 0.1, 2.2), "M_Black")
    k.box((0, 0.6, 1.1), (0.3, 0.1, 2.2), "M_Black")
    k.box((0, 0, 2.25), (0.3, 1.3, 0.1), "M_Black")
    k.box((0.0, 0, 1.1), (0.02, 1.1, 2.15), "M_LampWarm")              # luz al otro lado
    k.box((0.25, -0.2, 1.08), (0.05, 0.85, 2.12), "M_Wood", rot=(0, 0, -35))
    k.box((0.2, 0, 2.5), (0.1, 0.6, 0.08), "M_NeonAmber")
    return k


# ───────────────────────── piezas interactivas (con M_AccentGlow) ─────────────────────────
@asset
def SM_PowerConsole():
    k = Kit("SM_PowerConsole")
    k.box((0, 0, 0.55), (0.4, 0.55, 1.1), "M_DarkMetal")
    k.box((0.21, 0, 0.75), (0.02, 0.45, 0.5), "M_Black")
    for i in range(3):
        k.box((0.22, -0.13 + i * 0.13, 0.8), (0.03, 0.06, 0.18), "M_Steel")
    k.box((0.22, 0, 0.4), (0.03, 0.3, 0.08), "M_AccentGlow")
    k.box((0.25, 0.18, 0.62), (0.06, 0.04, 0.2), "M_Steel", rot=(0, 30, 0))   # palanca
    k.sphere((0.29, 0.18, 0.72), (0.035, 0.035, 0.035), "M_NeonRed", segs=6, rings=4)
    k.box((0, 0, 1.12), (0.44, 0.6, 0.04), "M_AccentGlow")
    return k


@asset
def SM_VentGrate():
    """Rejilla de conducto a ras de suelo 0.7 m (Y) x 0.12 m x 0.4 m; frente +X."""
    k = Kit("SM_VentGrate")
    k.box((0, 0, 0.2), (0.12, 0.7, 0.4), "M_Steel")
    k.box((0.04, 0, 0.2), (0.06, 0.6, 0.3), "M_Black")
    for i in range(5):
        k.box((0.07, 0, 0.08 + i * 0.06), (0.02, 0.6, 0.02), "M_Steel")
    k.box((0.07, 0, 0.38), (0.02, 0.7, 0.03), "M_AccentGlow")
    return k


@asset
def SM_FilmReelStand():
    k = Kit("SM_FilmReelStand")
    k.box((0, 0, 0.02), (0.3, 0.3, 0.04), "M_DarkMetal")
    k.box((0, 0, 0.2), (0.04, 0.04, 0.36), "M_DarkMetal")
    k.cyl((0, 0, 0.4), 0.17, 0.05, "M_Steel", segs=14, rot=(0, 90, 0))
    k.cyl((0, 0, 0.4), 0.13, 0.06, "M_Black", segs=14, rot=(0, 90, 0))
    k.torus((0, 0, 0.4), 0.175, 0.012, "M_AccentGlow", segs=14, ring=4, rot=(0, 90, 0))
    for i in range(3):
        a = i * 2 * math.pi / 3
        k.cyl((0.03, 0.08 * math.cos(a), 0.4 + 0.08 * math.sin(a)), 0.035, 0.07, "M_DarkMetal", segs=6, rot=(0, 90, 0))
    return k


@asset
def SM_Terminal():
    k = Kit("SM_Terminal")
    k.box((0, 0, 0.45), (0.45, 0.6, 0.9), "M_DarkMetal")
    k.box((0.08, 0, 1.1), (0.08, 0.6, 0.45), "M_DarkMetal", rot=(0, -20, 0))
    k.box((0.13, 0, 1.1), (0.01, 0.5, 0.36), "M_AccentGlow", rot=(0, -20, 0))
    k.box((0.2, 0, 0.88), (0.25, 0.55, 0.04), "M_Steel", rot=(0, 15, 0))
    k.box((0.23, 0, 0.3), (0.01, 0.4, 0.04), "M_AccentGlow")
    return k


@asset
def SM_DuctHatch():
    k = Kit("SM_DuctHatch")
    k.box((0, 0, 0.2), (0.12, 0.65, 0.4), "M_DarkMetal")
    k.box((0.065, 0, 0.2), (0.01, 0.55, 0.3), "M_Black")
    for i in range(4):
        k.box((0.07, -0.2 + i * 0.13, 0.2), (0.015, 0.03, 0.28), "M_Steel")
    k.box((0.07, 0, 0.37), (0.02, 0.65, 0.025), "M_AccentGlow")
    return k


@asset
def SM_AnchorFragment():
    """Cristal flotante con anillo de película (centrado: pivote en el centro)."""
    k = Kit("SM_AnchorFragment")
    k.ico((0, 0, 0.27), (0.1, 0.1, 0.27), "M_AccentGlow", subdiv=1)
    k.torus((0, 0, 0.27), 0.17, 0.012, "M_Gold", segs=16, ring=4, rot=(70, 0, 20))
    return k


@asset
def SM_ArchiveCore():
    k = Kit("SM_ArchiveCore")
    k.cyl((0, 0, 0.1), 0.42, 0.2, "M_StoneBlock", segs=8)
    k.cyl((0, 0, 0.4), 0.28, 0.45, "M_Black", segs=8)
    k.cyl((0, 0, 0.64), 0.34, 0.06, "M_Gold", segs=8)
    k.ico((0, 0, 0.95), (0.18, 0.18, 0.18), "M_AccentGlow", subdiv=2)
    k.torus((0, 0, 0.95), 0.3, 0.012, "M_Gold", segs=18, ring=4, rot=(60, 0, 0))
    k.torus((0, 0, 0.95), 0.34, 0.012, "M_Gold", segs=18, ring=4, rot=(-50, 30, 0))
    return k


@asset
def SM_EndingPedestal():
    k = Kit("SM_EndingPedestal")
    k.box((0, 0, 0.05), (0.6, 0.6, 0.1), "M_StoneBlock")
    k.cyl((0, 0, 0.4), 0.2, 0.6, "M_Black", segs=6)
    k.box((0, 0, 0.72), (0.5, 0.5, 0.05), "M_Gold")
    k.box((0, 0, 0.78), (0.42, 0.42, 0.04), "M_AccentGlow")
    k.cone((0, 0, 0.92), 0.08, 0.2, "M_AccentGlow", segs=4)
    return k


# ───────────────────────── personajes ─────────────────────────
@asset
def SM_Ivo():
    """Ivo, ~1.8 m, chaqueta marrón y camisa azul (mockup). Frente +X."""
    k = Kit("SM_Ivo")
    for y in (-0.11, 0.11):
        k.box((0.04, y, 0.05), (0.28, 0.12, 0.1), "M_Black")                    # zapatos
        k.cyl((0, y, 0.48), 0.075, 0.8, "M_Jeans", segs=8)                       # piernas
    k.box((0, 0, 0.92), (0.22, 0.38, 0.14), "M_Jeans")                          # cadera
    k.box((0, 0, 1.2), (0.26, 0.42, 0.48), "M_Jacket")                          # torso
    k.box((0.135, 0, 1.25), (0.01, 0.14, 0.36), "M_Shirt")                       # camisa abierta
    k.box((0, 0, 1.47), (0.24, 0.46, 0.08), "M_Jacket")                         # hombros
    for y, rot in ((-0.255, 4), (0.255, -4)):
        k.cyl((0.0, y, 1.18), 0.06, 0.55, "M_Jacket", segs=8, rot=(rot, 0, 0))   # brazos
        k.sphere((0.0, y * 1.06, 0.87), (0.05, 0.045, 0.06), "M_Skin", segs=6, rings=4)
    k.cyl((0, 0, 1.55), 0.055, 0.1, "M_Skin", segs=8)                           # cuello
    k.sphere((0.01, 0, 1.69), (0.11, 0.1, 0.13), "M_Skin", segs=10, rings=7)    # cabeza
    k.sphere((-0.015, 0, 1.75), (0.115, 0.108, 0.1), "M_Hair", segs=10, rings=6)
    k.box((0.07, 0, 1.79), (0.08, 0.2, 0.05), "M_Hair", rot=(0, -20, 0))        # flequillo
    for y in (-0.04, 0.04):
        k.sphere((0.1, y, 1.7), (0.012, 0.012, 0.014), "M_Black", segs=6, rings=4)
    k.box((0.115, 0, 1.66), (0.03, 0.025, 0.04), "M_Skin")                      # nariz
    return k


@asset
def SM_Nilo():
    """Nilo: gato esmoquin (negro con pecho y patas blancas), collar rojo y campanilla. Frente +X."""
    k = Kit("SM_Nilo")
    k.sphere((0, 0, 0.24), (0.24, 0.11, 0.11), "M_CatBlack", segs=12, rings=8)        # cuerpo
    k.sphere((0.14, 0, 0.22), (0.09, 0.085, 0.1), "M_CatWhite", segs=10, rings=6)      # pecho
    for x in (0.14, -0.14):
        for y in (0.06, -0.06):
            front = x > 0
            k.cyl((x, y, 0.09), 0.028, 0.18, "M_CatBlack", segs=6)
            k.sphere((x + 0.01, y, 0.02), (0.035, 0.028, 0.02), "M_CatWhite" if front else "M_CatBlack", segs=6, rings=4)
    k.sphere((0.27, 0, 0.38), (0.1, 0.095, 0.09), "M_CatBlack", segs=12, rings=8)      # cabeza
    k.sphere((0.345, 0, 0.35), (0.04, 0.05, 0.035), "M_CatWhite", segs=8, rings=5)     # hocico
    k.sphere((0.383, 0, 0.365), (0.012, 0.014, 0.01), "M_CatPink", segs=6, rings=4)    # nariz
    for y in (-0.04, 0.04):
        k.sphere((0.345, y, 0.4), (0.016, 0.022, 0.022), "M_CatEye", segs=8, rings=5)  # ojos ámbar
        k.sphere((0.358, y, 0.4), (0.006, 0.006, 0.016), "M_Black", segs=6, rings=4)
        k.cone((0.25, y * 1.4, 0.47), 0.04, 0.08, "M_CatBlack", segs=4, rot=(-10 * (1 if y > 0 else -1), 0, 45))
        k.cone((0.262, y * 1.35, 0.465), 0.022, 0.05, "M_CatPink", segs=4, rot=(-10 * (1 if y > 0 else -1), 0, 45))
    k.torus((0.2, 0, 0.32), 0.075, 0.012, "M_Collar", segs=14, ring=4, rot=(0, -60, 0))  # collar
    k.sphere((0.26, 0, 0.265), (0.018, 0.018, 0.018), "M_Gold", segs=8, rings=5)          # campanilla
    # cola curvada hacia arriba
    pts = [(-0.22, 0, 0.27), (-0.3, 0, 0.32), (-0.35, 0, 0.4), (-0.37, 0, 0.5), (-0.35, 0, 0.58)]
    for (x0, y0, z0), (x1, y1, z1) in zip(pts, pts[1:]):
        mid = ((x0 + x1) / 2, 0, (z0 + z1) / 2)
        length = math.dist((x0, z0), (x1, z1))
        ang = math.degrees(math.atan2(x1 - x0, z1 - z0))
        k.cyl(mid, 0.022, length + 0.02, "M_CatBlack", segs=6, rot=(0, ang, 0))
    k.sphere(pts[-1], (0.024, 0.024, 0.024), "M_CatWhite", segs=6, rings=4)
    return k


@asset
def SM_Watcher():
    """Centinela flotante (enemigo tipo 1): cuerpo oscuro y visor rojo; frente +X. Pivote en el centro."""
    k = Kit("SM_Watcher")
    k.cyl((0, 0, 0.0), 0.32, 0.9, "M_Black", segs=8, radius2=0.22)
    k.sphere((0, 0, 0.55), (0.3, 0.3, 0.25), "M_DarkMetal", segs=10, rings=6)
    k.box((0.24, 0, 0.55), (0.1, 0.4, 0.08), "M_AccentGlow")           # visor (se tiñe)
    k.torus((0, 0, -0.1), 0.36, 0.025, "M_AccentGlow", segs=16, ring=4)
    k.cone((0, 0, -0.62), 0.2, 0.35, "M_DarkMetal", segs=8, rot=(180, 0, 0))
    k.cyl((0, 0, 0.9), 0.012, 0.3, "M_Steel", segs=6)
    k.sphere((0, 0, 1.06), (0.03, 0.03, 0.03), "M_AccentGlow", segs=6, rings=4)
    return k


# ───────────────────────── exportación ─────────────────────────
def main():
    report = {}
    for name, fn in ASSETS.items():
        bpy.ops.wm.read_factory_settings(use_empty=True)
        kit = fn()
        obj = kit.build()
        bb = [obj.matrix_world @ v.co for v in obj.data.vertices]
        dims = [round(max(c[i] for c in bb) - min(c[i] for c in bb), 3) for i in range(3)]
        export(obj, os.path.join(MESH_DIR, name + ".fbx"))
        mn = [min(c[i] for c in bb) for i in range(3)]
        mx = [max(c[i] for c in bb) for i in range(3)]
        # bounds en espacio Unreal (cm, Y invertida respecto a Blender)
        ue_bounds = [[round(mn[0] * 100, 1), round(-mx[1] * 100, 1), round(mn[2] * 100, 1)],
                     [round(mx[0] * 100, 1), round(-mn[1] * 100, 1), round(mx[2] * 100, 1)]]
        report[name] = {"faces": len(obj.data.polygons), "size_m": dims, "bounds_cm": ue_bounds, "materials": kit.mats}
        print(f"EXPORTED {name} faces={len(obj.data.polygons)} size={dims}")
    used = sorted({m for r in report.values() for m in r["materials"]})
    manifest = {
        "materials": {m: {"color": MATERIALS[m][0], "roughness": MATERIALS[m][1], "metallic": MATERIALS[m][2],
                          "emissive": MATERIALS[m][3], "texture": MATERIALS[m][4]} for m in used},
        "meshes": report,
    }
    with open(os.path.join(OUT, "kit_manifest.json"), "w") as f:
        json.dump(manifest, f, indent=2)
    print("TOTAL", len(report), "assets,", sum(r["faces"] for r in report.values()), "faces")


main()
