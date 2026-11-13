"""Fuente única del decorado de The Unfinished Cut.

Genera:
  - Source/TheUnfinishedCut/TUCLayoutData.h  (tablas C++ que usa ATUCBlockoutBuilder)
  - layout.json                              (para el render de verificación en Blender)

Coordenadas en unidades de Unreal (cm, X adelante, Y derecha, Z arriba). Suelo a Z = -5.
Los assets miran a +X; Yaw en grados (Unreal).
"""
import json
import math
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
PROJECT = os.path.abspath(os.path.join(HERE, "..", ".."))
KIT = os.path.join(PROJECT, "ExternalAssets", "TUC_Kit")
MANIFEST = json.load(open(os.path.join(KIT, "kit_manifest.json")))
FLOOR_Z = -5.0

visuals = []    # (asset, x, y, z, yaw, sx, sy, sz)
colliders = []  # (name, cx, cy, cz, hx, hy, hz, r, g, b)
lights = []     # (x, y, z, r, g, b, intensity, radius)
signs = []      # (text, x, y, z, yaw, size, r, g, b)


def size_cm(asset):
    s = MANIFEST["meshes"][asset]["size_m"]
    return [v * 100 for v in s]


def bounds_cm(asset):
    """Bounds locales (min, max) del asset en cm, leídos del FBX exportado."""
    return MANIFEST["meshes"][asset]["bounds_cm"]


def vis(asset, x, y, z=FLOOR_Z, yaw=0.0, scale=(1, 1, 1)):
    visuals.append((asset, x, y, z, yaw, *scale))


def prop(asset, x, y, z=FLOOR_Z, yaw=0.0, scale=(1, 1, 1), collide=True, color=(0.2, 0.2, 0.25), pad=0.0):
    vis(asset, x, y, z, yaw, scale)
    if not collide:
        return
    (mnx, mny, mnz), (mxx, mxy, mxz) = bounds_cm(asset)
    mnx, mxx = mnx * scale[0], mxx * scale[0]
    mny, mxy = mny * scale[1], mxy * scale[1]
    mnz, mxz = mnz * scale[2], mxz * scale[2]
    # yaw múltiplo de 90: rotar la caja
    corners = [(mnx, mny), (mnx, mxy), (mxx, mny), (mxx, mxy)]
    a = math.radians(yaw)
    rc = [(cx * math.cos(a) - cy * math.sin(a), cx * math.sin(a) + cy * math.cos(a)) for cx, cy in corners]
    x0, x1 = min(c[0] for c in rc), max(c[0] for c in rc)
    y0, y1 = min(c[1] for c in rc), max(c[1] for c in rc)
    colliders.append((asset, x + (x0 + x1) / 2, y + (y0 + y1) / 2, z + (mnz + mxz) / 2,
                      (x1 - x0) / 2 + pad, (y1 - y0) / 2 + pad, (mxz - mnz) / 2, *color))


def tiles(asset_fn, x0, x1, y0, y1):
    x = x0 + 50
    while x < x1:
        y = y0 + 50
        while y < y1:
            vis(asset_fn(x, y), x, y, FLOOR_Z)
            y += 100
        x += 100


def wall_run(asset, axis, fixed, start, end, yaw):
    """Fila de segmentos de 1 m. axis='x': recorre X con Y fijo; 'y': recorre Y con X fijo."""
    p = start + 50
    while p < end + 1:
        if axis == "x":
            vis(asset, p, fixed, FLOOR_Z, yaw)
        else:
            vis(asset, fixed, p, FLOOR_Z, yaw)
        p += 100


def room(x0, x1, floor, wall, start=True, end=True):
    tiles(lambda x, y: floor, x0, x1, -500, 500)
    wall_run(wall, "x", -485, x0, x1, 90)
    wall_run(wall, "x", 485, x0, x1, -90)
    if start:
        wall_run(wall, "y", x0 + 15, -500, 500, 0)
    if end:
        wall_run(wall, "y", x1 - 15, -500, 500, 180)


# ───────────── ACTO I: calle ─────────────
tiles(lambda x, y: "SM_Floor_Asphalt" if abs(y) < 300 else "SM_Floor_Sidewalk", -1250, -150, -600, 600)
tiles(lambda x, y: "SM_Floor_Sidewalk", -150, 0, -600, 600)
for x in (-975, -425):
    prop("SM_Building_A", x, -600, yaw=90, collide=False)
    prop("SM_Building_A", x, 600, yaw=-90, collide=False)
for y in (-300, 300):
    prop("SM_Building_B", -1250, y, yaw=0, collide=False)
for x in (-100, -40):
    vis("SM_Wall_Brick", x, -610, FLOOR_Z, 90)
    vis("SM_Wall_Brick", x, 610, FLOOR_Z, -90)
prop("SM_Van", -700, 420, yaw=0, color=(0.4, 0.05, 0.05))
prop("SM_Barrier", -520, -260, yaw=90, color=(0.6, 0.3, 0.05))
prop("SM_Barrier", -280, 300, yaw=90, color=(0.6, 0.3, 0.05))
for x in (-1000, -400):
    for side, yaw in ((-1, 90), (1, -90)):
        prop("SM_StreetLamp", x, side * 540, yaw=yaw, collide=False)
        colliders.append(("SM_StreetLamp", x, side * 540, 215.0, 16.0, 16.0, 220.0, 0.05, 0.05, 0.06))
    # farol a ~1.9 m del poste (modelo Meshy) o 0.85 m (kit)
    reach = 190 if MANIFEST["meshes"]["SM_StreetLamp"].get("override") else 85
    lights.append((x, -540 + reach, 330, 1.0, 0.75, 0.45, 3500, 700))
    lights.append((x, 540 - reach, 330, 1.0, 0.75, 0.45, 3500, 700))
prop("SM_Hydrant", -620, -545, color=(0.4, 0.05, 0.05))
prop("SM_TrashBin", -220, -540, color=(0.03, 0.12, 0.16))
lights.append((-700, 0, 280, 0.05, 0.2, 0.7, 4200, 950))
lights.append((-130, 0, 430, 0.95, 0.03, 0.12, 6500, 850))

# ───────────── ACTO I: cine ─────────────
tiles(lambda x, y: "SM_Floor_Carpet", 0, 1450, -500, 500)
wall_run("SM_Wall_Cinema", "x", -485, 0, 1450, 90)
wall_run("SM_Wall_Cinema", "x", 485, 0, 1450, -90)
wall_run("SM_Wall_Cinema", "y", 1435, -500, 500, 180)
vis("SM_CinemaFacadeSide", 0, -400, FLOOR_Z, 180)
vis("SM_CinemaFacadeSide", 0, 400, FLOOR_Z, 180)
vis("SM_CinemaMarquee", 0, 0, FLOOR_Z, 180)
signs.append(("ETERNAL WAVE", -153, 0, 397, 180, 44, 255, 50, 70))
signs.append(("EL 2º FILM DE NEO · LOS REINOS INTERIORES", -28, 0, 330, 180, 15, 255, 220, 190))
prop("SM_ConcessionCounter", 700, -250, yaw=90, color=(0.25, 0.12, 0.06))
prop("SM_LobbyPartition", 520, 220, yaw=0, color=(0.25, 0.12, 0.06))
if "SM_PopcornCart" in MANIFEST["meshes"]:
    prop("SM_PopcornCart", 900, -420, yaw=90, color=(0.5, 0.05, 0.05))
prop("SM_SeatRow", 1000, 245, yaw=0, color=(0.3, 0.02, 0.05))
prop("SM_SeatRow", 1150, 245, yaw=0, color=(0.3, 0.02, 0.05))
prop("SM_ProjectionScreen", 1400, 0, yaw=180, color=(0.6, 0.6, 0.65))
lights += [(220, 0, 250, 0.08, 0.3, 1.0, 5000, 850), (900, 0, 250, 0.65, 0.05, 0.18, 4200, 750),
           (1300, 0, 450, 0.9, 0.85, 0.8, 3500, 700)]

# ───────────── ACTO II: años 80 ─────────────
room(1650, 2950, "SM_Floor_Checker", "SM_Wall_Retro")
for x in (2000, 2080, 2160):
    prop("SM_ArcadeCabinet", x, -430, yaw=90, color=(0.12, 0.03, 0.2))
for x in (2250, 2330):
    prop("SM_ArcadeCabinet", x, 430, yaw=-90, color=(0.12, 0.03, 0.2))
prop("SM_Jukebox", 1720, -400, yaw=0, color=(0.25, 0.12, 0.06))
prop("SM_Stage", 2700, 0, yaw=0, color=(0.15, 0.02, 0.1))
prop("SM_Speaker", 2820, -140, z=FLOOR_Z + 31, yaw=180, color=(0.02, 0.02, 0.02))
prop("SM_Speaker", 2820, 140, z=FLOOR_Z + 31, yaw=180, color=(0.02, 0.02, 0.02))
signs.append(("ARCADE", 2918, 0, 380, 180, 90, 255, 60, 220))
lights += [(1950, -200, 240, 1.0, 0.02, 0.55, 6500, 850), (2550, 220, 240, 0.0, 0.85, 1.0, 6500, 850)]

# ───────────── ACTO III: futuro ─────────────
room(3000, 4400, "SM_Floor_Metal", "SM_Wall_Future")
prop("SM_SciFiTower", 3500, -300, color=(0.05, 0.06, 0.08))
prop("SM_SciFiTower", 3950, 270, color=(0.05, 0.06, 0.08))
prop("SM_DuctFrame", 3900, 460, yaw=-90, color=(0.06, 0.06, 0.08))
prop("SM_FutureCrate", 4180, -425, color=(0.03, 0.12, 0.16))
prop("SM_FutureCrate", 4275, -425, color=(0.03, 0.12, 0.16))
prop("SM_FutureCrate", 4228, -425, z=FLOOR_Z + 80, yaw=45, collide=False)
vis("SM_HoloPanel", 3300, -472, FLOOR_Z + 150, 90)
vis("SM_HoloPanel", 3600, 472, FLOOR_Z + 170, -90)
vis("SM_HoloPanel", 4371, 250, FLOOR_Z + 170, 180)
lights += [(3300, 0, 270, 0.0, 0.5, 1.0, 7000, 900), (4100, 0, 270, 0.5, 0.02, 1.0, 7000, 900)]

# ───────────── ACTO IV: Archivo de los Ecos ─────────────
room(4450, 5750, "SM_Floor_Stone", "SM_Wall_Archive")
prop("SM_Monolith", 4800, -260, color=(0.07, 0.04, 0.13))
prop("SM_Monolith", 4800, 260, color=(0.07, 0.04, 0.13))
for x in (4600, 5300):
    prop("SM_ReelShelf", x, -455, yaw=90, color=(0.25, 0.12, 0.06))
    prop("SM_ReelShelf", x, 455, yaw=-90, color=(0.25, 0.12, 0.06))
prop("SM_ProjectionScreen", 5700, 0, yaw=180, color=(0.6, 0.6, 0.65))
prop("SM_PrologueDoor", 5620, 0, yaw=180, color=(0.02, 0.02, 0.02))
signs.append(("CORTE 001 — FINAL REGISTRADO", 5655, 0, 520, 180, 24, 255, 210, 120))
lights += [(4900, 0, 260, 0.2, 0.0, 1.0, 8000, 1000), (5450, 0, 250, 1.0, 0.7, 0.1, 8500, 900)]

# ───────────── piezas interactivas (solo para el render; el C++ las coloca) ─────────────
INTERACTABLES = [
    ("SM_PowerConsole", 260, -422, 90), ("SM_VentGrate", 900, 466, -90), ("PROJECTOR", 1080, -120, 0),
    ("SM_FilmReelStand", 1900, 350, -90), ("PROJECTOR", 2740, 0, 0), ("SM_AnchorFragment", 2220, -350, 0),
    ("SM_Terminal", 3260, -447, 90), ("SM_DuctHatch", 3900, 438, -90), ("PROJECTOR", 4300, -120, 0),
    ("SM_AnchorFragment", 3600, 380, 0), ("SM_AnchorFragment", 4760, -350, 0), ("SM_ArchiveCore", 5000, 0, 0),
    ("SM_EndingPedestal", 5400, -260, 180), ("SM_EndingPedestal", 5400, 0, 180), ("SM_EndingPedestal", 5400, 260, 180),
]


def cstr(s):
    return 'TEXT("' + s.replace('\\', '\\\\').replace('"', '\\"') + '")'


def f(v):
    return f"{v:.1f}f"


def write_header(path):
    lines = ["﻿// GENERADO por Tools/Kit/layout.py — no editar a mano. Regenerar con: python layout.py",
             "#pragma once", "", '#include "CoreMinimal.h"', "",
             "struct FTUCLayoutVisual { const TCHAR* Asset; float X, Y, Z, Yaw, SX, SY, SZ; };",
             "struct FTUCLayoutCollider { const TCHAR* Name; float X, Y, Z, HX, HY, HZ, R, G, B; };",
             "struct FTUCLayoutLight { float X, Y, Z, R, G, B, Intensity, Radius; };",
             "struct FTUCLayoutSign { const TCHAR* Text; float X, Y, Z, Yaw, Size; uint8 R, G, B; };", "",
             "static const FTUCLayoutVisual GTUCLayoutVisuals[] =", "{"]
    for a, x, y, z, yaw, sx, sy, sz in visuals:
        lines.append(f"    {{ {cstr(a)}, {f(x)}, {f(y)}, {f(z)}, {f(yaw)}, {f(sx)}, {f(sy)}, {f(sz)} }},")
    lines += ["};", "", "static const FTUCLayoutCollider GTUCLayoutColliders[] =", "{"]
    for n, x, y, z, hx, hy, hz, r, g, b in colliders:
        lines.append(f"    {{ {cstr(n)}, {f(x)}, {f(y)}, {f(z)}, {f(hx)}, {f(hy)}, {f(hz)}, {r:.2f}f, {g:.2f}f, {b:.2f}f }},")
    lines += ["};", "", "static const FTUCLayoutLight GTUCLayoutLights[] =", "{"]
    for x, y, z, r, g, b, i, rad in lights:
        lines.append(f"    {{ {f(x)}, {f(y)}, {f(z)}, {r:.2f}f, {g:.2f}f, {b:.2f}f, {f(i)}, {f(rad)} }},")
    lines += ["};", "", "static const FTUCLayoutSign GTUCLayoutSigns[] =", "{"]
    for t, x, y, z, yaw, size, r, g, b in signs:
        lines.append(f"    {{ {cstr(t)}, {f(x)}, {f(y)}, {f(z)}, {f(yaw)}, {f(size)}, {r}, {g}, {b} }},")
    lines += ["};", ""]
    with open(path, "w", encoding="utf-8") as fh:
        fh.write("\n".join(lines))


if __name__ == "__main__":
    header = sys.argv[1] if len(sys.argv) > 1 else os.path.join(PROJECT, "Source", "TheUnfinishedCut", "TUCLayoutData.h")
    write_header(header)
    json.dump({"visuals": visuals, "colliders": colliders, "lights": lights, "signs": signs, "interactables": INTERACTABLES},
              open(os.path.join(KIT, "layout.json"), "w"))
    print(f"layout: {len(visuals)} visuales, {len(colliders)} colisiones, {len(lights)} luces, {len(signs)} letreros")
