"""Genera las texturas tileables del kit TUC (512 px, sin texto, CC0 propias)."""
import sys, os, random
import numpy as np
from PIL import Image, ImageDraw, ImageFilter

OUT = sys.argv[1]
os.makedirs(OUT, exist_ok=True)
S = 512
rng = np.random.default_rng(7)


def noise(scale=8, octaves=4, seed=0):
    r = np.random.default_rng(seed)
    acc = np.zeros((S, S))
    amp = 1.0
    for o in range(octaves):
        n = scale * (2 ** o)
        g = r.random((n, n))
        img = Image.fromarray((g * 255).astype(np.uint8)).resize((S, S), Image.BICUBIC)
        # tileable: wrap by blending with rolled copy
        a = np.asarray(img, dtype=float) / 255
        acc += a * amp
        amp *= 0.5
    acc -= acc.min(); acc /= acc.max()
    return acc


def save(name, arr):
    arr = np.clip(arr, 0, 1)
    Image.fromarray((arr * 255).astype(np.uint8)).save(os.path.join(OUT, name + ".png"))


def tint(gray, c0, c1):
    g = gray[..., None]
    return np.array(c0) * (1 - g) + np.array(c1) * g


def make_tileable(arr):
    # mezcla con versiones desplazadas para eliminar costuras
    h = S // 2
    w = np.minimum(np.arange(S), S - 1 - np.arange(S)) / h  # 0 en bordes, 1 centro
    wx = np.clip(w, 0, 1)[None, :]
    wy = np.clip(w, 0, 1)[:, None]
    rolled = np.roll(np.roll(arr, h, 0), h, 1)
    m = (wx * wy)
    if arr.ndim == 3:
        m = m[..., None]
    return arr * m + rolled * (1 - m)


# --- Asfalto ---
n = make_tileable(noise(16, 4, 1))
fine = rng.random((S, S)) * 0.25
asphalt = tint(n * 0.6 + fine, (0.07, 0.075, 0.09), (0.2, 0.2, 0.22))
save("T_Asphalt", asphalt)

# --- Acera (losas) ---
side = tint(make_tileable(noise(8, 3, 2)) * 0.5 + rng.random((S, S)) * 0.15, (0.22, 0.22, 0.25), (0.36, 0.36, 0.38))
img = Image.fromarray((side * 255).astype(np.uint8)); d = ImageDraw.Draw(img)
for k in range(0, S + 1, S // 2):
    d.line([(k, 0), (k, S)], fill=(40, 40, 46), width=6); d.line([(0, k), (S, k)], fill=(40, 40, 46), width=6)
img.save(os.path.join(OUT, "T_Sidewalk.png"))

# --- Alfombra de cine (rombos dorados sobre rojo) ---
img = Image.new("RGB", (S, S), (92, 10, 22)); d = ImageDraw.Draw(img)
step = S // 4
for i in range(-1, 5):
    for j in range(-1, 5):
        cx, cy = i * step + (step // 2 if j % 2 else 0), j * step
        d.polygon([(cx, cy - step // 2 + 6), (cx + step // 2 - 6, cy), (cx, cy + step // 2 - 6), (cx - step // 2 + 6, cy)], outline=(190, 140, 40), width=5)
        d.ellipse([cx - 8, cy - 8, cx + 8, cy + 8], fill=(170, 30, 50))
arr = np.asarray(img, dtype=float) / 255
arr *= (0.85 + 0.15 * rng.random((S, S)))[..., None]
save("T_Carpet", arr)

# --- Damero años 80 ---
img = Image.new("RGB", (S, S), (12, 8, 20)); d = ImageDraw.Draw(img)
h = S // 2
d.rectangle([0, 0, h - 1, h - 1], fill=(225, 220, 235)); d.rectangle([h, h, S - 1, S - 1], fill=(225, 220, 235))
arr = np.asarray(img, dtype=float) / 255
arr = arr * (0.9 + 0.1 * make_tileable(noise(8, 3, 3)))[..., None]
save("T_Checker", arr)

# --- Paneles metálicos (futuro) ---
base = tint(make_tileable(noise(8, 3, 4)) * 0.4 + rng.random((S, S)) * 0.08, (0.16, 0.19, 0.23), (0.30, 0.34, 0.40))
img = Image.fromarray((base * 255).astype(np.uint8)); d = ImageDraw.Draw(img)
for k in (0, S // 2):
    d.rectangle([k + 2, 2, k + S // 2 - 3, S // 2 - 3], outline=(20, 24, 30), width=6)
    d.rectangle([k + 2, S // 2 + 2, k + S // 2 - 3, S - 3], outline=(20, 24, 30), width=6)
for x in (24, S // 2 - 24, S // 2 + 24, S - 24):
    for y in (24, S // 2 - 24, S // 2 + 24, S - 24):
        d.ellipse([x - 6, y - 6, x + 6, y + 6], fill=(120, 130, 145))
img.save(os.path.join(OUT, "T_MetalPanel.png"))

# --- Losas de piedra oscura (archivo) ---
img = Image.new("RGB", (S, S)); px = np.zeros((S, S, 3))
n = make_tileable(noise(12, 4, 5))
px[:] = tint(n * 0.5, (0.07, 0.06, 0.10), (0.18, 0.16, 0.24))
img = Image.fromarray((px * 255).astype(np.uint8)); d = ImageDraw.Draw(img)
for k in (0, S // 2, S):
    d.line([(k, 0), (k, S)], fill=(8, 6, 12), width=8); d.line([(0, k), (S, k)], fill=(8, 6, 12), width=8)
img.save(os.path.join(OUT, "T_StoneTile.png"))

# --- Ladrillo (calle) ---
img = Image.new("RGB", (S, S), (30, 30, 38)); d = ImageDraw.Draw(img)
bh, bw = S // 8, S // 4
for row in range(8):
    off = (bw // 2) if row % 2 else 0
    for col in range(-1, 5):
        x0 = col * bw + off; y0 = row * bh
        v = random.Random(row * 31 + col).uniform(0.75, 1.15)
        c = tuple(int(ch * v) for ch in (78, 52, 58))
        d.rectangle([x0 + 3, y0 + 3, x0 + bw - 3, y0 + bh - 3], fill=c)
arr = np.asarray(img, dtype=float) / 255 * (0.85 + 0.15 * rng.random((S, S)))[..., None]
save("T_Brick", arr)

# --- Paneles de terciopelo (cine) ---
x = np.arange(S)
groove = (np.abs(((x % (S // 4)) - S // 8)) / (S // 8)) ** 0.5
arr = tint(np.tile(groove, (S, 1)) * 0.7 + rng.random((S, S)) * 0.08, (0.12, 0.015, 0.03), (0.34, 0.05, 0.08))
save("T_CinemaPanel", arr)

# --- Pared retro (rejilla synthwave) ---
img = Image.new("RGB", (S, S), (22, 6, 34)); d = ImageDraw.Draw(img)
for k in range(0, S, S // 4):
    d.line([(k, 0), (k, S)], fill=(90, 20, 120), width=3); d.line([(0, k), (S, k)], fill=(90, 20, 120), width=3)
img = img.filter(ImageFilter.GaussianBlur(1.2))
img.save(os.path.join(OUT, "T_RetroGrid.png"))

# --- Piedra en bloques (archivo, paredes) ---
img = Image.fromarray((tint(make_tileable(noise(10, 4, 6)) * 0.6, (0.06, 0.05, 0.09), (0.16, 0.14, 0.21)) * 255).astype(np.uint8)); d = ImageDraw.Draw(img)
for row in range(4):
    y0 = row * S // 4; off = S // 4 if row % 2 else 0
    d.line([(0, y0), (S, y0)], fill=(6, 5, 9), width=6)
    for col in range(-1, 3):
        xk = col * S // 2 + off; d.line([(xk, y0), (xk, y0 + S // 4)], fill=(6, 5, 9), width=6)
img.save(os.path.join(OUT, "T_StoneBlock.png"))

# --- Ventanas de edificio (emisivas) ---
img = Image.new("RGB", (S, S), (10, 12, 20)); d = ImageDraw.Draw(img)
r = random.Random(11)
for row in range(4):
    for col in range(4):
        x0, y0 = col * S // 4 + 22, row * S // 4 + 18
        lit = r.random() < 0.55
        c = r.choice([(255, 200, 110), (255, 170, 90), (140, 200, 255)]) if lit else (24, 28, 40)
        d.rectangle([x0, y0, x0 + S // 4 - 44, y0 + S // 4 - 36], fill=c)
img.save(os.path.join(OUT, "T_Windows.png"))

# --- Pantalla de arcade ---
img = Image.new("RGB", (S, S), (5, 5, 20)); d = ImageDraw.Draw(img)
for k in range(0, S, 8):
    d.line([(0, k), (S, k)], fill=(0, 0, 0), width=2)
for i in range(18):
    x0, y0 = r.randint(20, S - 60), r.randint(20, S - 60)
    d.rectangle([x0, y0, x0 + r.randint(16, 50), y0 + 16], fill=r.choice([(255, 60, 200), (60, 230, 255), (255, 230, 60)]))
d.polygon([(S // 2, S - 60), (S // 2 - 30, S - 20), (S // 2 + 30, S - 20)], fill=(80, 255, 120))
img.save(os.path.join(OUT, "T_ArcadeScreen.png"))

# --- Tira de película (emisiva, archivo) ---
img = Image.new("RGB", (S, S), (6, 6, 8)); d = ImageDraw.Draw(img)
for k in range(0, S, S // 8):
    d.rectangle([12, k + 10, 44, k + 40], fill=(255, 200, 120)); d.rectangle([S - 44, k + 10, S - 12, k + 40], fill=(255, 200, 120))
for k in range(0, S, S // 3):
    d.rectangle([70, k + 14, S - 70, k + S // 3 - 14], fill=(r.randint(60, 140), r.randint(40, 90), r.randint(120, 220)))
img.save(os.path.join(OUT, "T_FilmStrip.png"))

# --- Pósters (formas abstractas, sin texto ni marcas) ---
for idx, (bg, fg) in enumerate([((20, 10, 40), (255, 80, 60)), ((10, 30, 50), (80, 220, 255)), ((40, 10, 20), (255, 200, 60))]):
    img = Image.new("RGB", (S // 2, S), bg); d = ImageDraw.Draw(img)
    d.ellipse([40, 120, 216, 296], fill=fg)
    d.polygon([(0, S), (128, 300), (256, S)], fill=tuple(int(c * 0.5) for c in fg))
    d.rectangle([30, 40, 226, 70], fill=tuple(min(255, c + 60) for c in bg))
    d.rectangle([30, S - 80, 226, S - 60], fill=tuple(min(255, c + 60) for c in bg))
    img.save(os.path.join(OUT, f"T_Poster{idx}.png"))

# --- Blanco (para materiales de color liso) ---
Image.new("RGB", (8, 8), (255, 255, 255)).save(os.path.join(OUT, "T_White.png"))
print("textures ok", sorted(os.listdir(OUT)))
