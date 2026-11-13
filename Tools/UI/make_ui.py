"""Genera las imágenes de la interfaz (menú, botones, cómo jugar, prólogo, pausa).

Estilo: terror acogedor (noche violeta, luna, niebla, huellas fantasma, cinta de cine).
Las imágenes llevan el texto ya dibujado, así siempre se ven en Unreal aunque falle una fuente.

Uso:  python3 Tools/UI/make_ui.py      (escribe en ExternalAssets/TUC_UI)
Fuentes: Luckiest Guy (Apache 2.0) y Lilita One (OFL), de Google Fonts, en ExternalAssets/TUC_UI/Fonts.
"""
import json
import math
import random
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter, ImageFont

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "ExternalAssets" / "TUC_UI"
FONTS = OUT / "Fonts"
W, H = 1920, 1080

INK = (16, 9, 34)
MOON = (255, 240, 196)
MINT = (140, 255, 214)
PUMPKIN = (255, 160, 60)
LILAC = (196, 170, 255)
WHITE = (250, 246, 255)
IVO = (90, 210, 255)
NILO = (110, 255, 140)
RED = (255, 80, 90)


def griffy(size):
    """Fuente de títulos: Luckiest Guy (Apache 2.0), gruesa y de videojuego."""
    return ImageFont.truetype(str(FONTS / "LuckiestGuy-Regular.ttf"), size)


def fredoka(size, weight="Bold"):
    """Fuente de interfaz: Lilita One (OFL)."""
    return ImageFont.truetype(str(FONTS / "LilitaOne-Regular.ttf"), size)


def text_size(draw, text, font):
    box = draw.textbbox((0, 0), text, font=font)
    return box[2] - box[0], box[3] - box[1], box


def glow_text(img, xy, text, font, fill, glow=(255, 220, 140), glow_radius=14, outline=INK, outline_w=5, anchor="la"):
    layer = Image.new("RGBA", img.size, (0, 0, 0, 0))
    d = ImageDraw.Draw(layer)
    d.text(xy, text, font=font, fill=glow + (200,), anchor=anchor, stroke_width=outline_w + 4, stroke_fill=glow + (200,))
    layer = layer.filter(ImageFilter.GaussianBlur(glow_radius))
    img.alpha_composite(layer)
    d = ImageDraw.Draw(img)
    d.text(xy, text, font=font, fill=fill, anchor=anchor, stroke_width=outline_w, stroke_fill=outline)


def wrap(draw, text, font, max_w):
    words, lines, cur = text.split(), [], ""
    for word in words:
        cand = (cur + " " + word).strip()
        if draw.textlength(cand, font=font) > max_w and cur:
            lines.append(cur)
            cur = word
        else:
            cur = cand
    if cur:
        lines.append(cur)
    return lines


def draw_wrapped(draw, xy, text, font, fill, max_w, gap=8):
    x, y = xy
    for line in wrap(draw, text, font, max_w):
        draw.text((x, y), line, font=font, fill=fill)
        y += font.size + gap
    return y


def rounded_panel(img, box, fill, outline=None, radius=28, width=3, shadow=True):
    if shadow:
        sh = Image.new("RGBA", img.size, (0, 0, 0, 0))
        ImageDraw.Draw(sh).rounded_rectangle((box[0] + 8, box[1] + 12, box[2] + 8, box[3] + 12), radius, fill=(0, 0, 0, 140))
        img.alpha_composite(sh.filter(ImageFilter.GaussianBlur(12)))
    layer = Image.new("RGBA", img.size, (0, 0, 0, 0))
    ImageDraw.Draw(layer).rounded_rectangle(box, radius, fill=fill, outline=outline, width=width)
    img.alpha_composite(layer)


def paw(draw, cx, cy, s, fill):
    draw.ellipse((cx - s * 0.55, cy - s * 0.2, cx + s * 0.55, cy + s * 0.6), fill=fill)
    for dx, dy in ((-0.62, -0.45), (-0.22, -0.78), (0.22, -0.78), (0.62, -0.45)):
        r = s * 0.2
        draw.ellipse((cx + dx * s - r, cy + dy * s - r, cx + dx * s + r, cy + dy * s + r), fill=fill)


def night_background(seed=3, moon=(1500, 250), moon_r=150):
    random.seed(seed)
    img = Image.new("RGBA", (W, H))
    px = img.load()
    for y in range(H):
        t = y / H
        r = int(18 + 30 * t)
        g = int(10 + 12 * t)
        b = int(40 + 50 * t)
        for x in range(W):
            px[x, y] = (r, g, b, 255)
    # resplandor violeta
    glow = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    gd = ImageDraw.Draw(glow)
    gd.ellipse((moon[0] - 700, moon[1] - 500, moon[0] + 700, moon[1] + 700), fill=(120, 70, 200, 90))
    img.alpha_composite(glow.filter(ImageFilter.GaussianBlur(160)))
    d = ImageDraw.Draw(img)
    for _ in range(260):
        x, y = random.randint(0, W), random.randint(0, int(H * 0.7))
        s = random.choice((1, 1, 1, 2, 2, 3))
        a = random.randint(120, 255)
        d.ellipse((x, y, x + s, y + s), fill=(255, 250, 230, a))
    # luna
    halo = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    hd = ImageDraw.Draw(halo)
    for k, a in ((2.4, 40), (1.7, 70), (1.25, 110)):
        hd.ellipse((moon[0] - moon_r * k, moon[1] - moon_r * k, moon[0] + moon_r * k, moon[1] + moon_r * k), fill=(255, 230, 170, a))
    img.alpha_composite(halo.filter(ImageFilter.GaussianBlur(50)))
    d = ImageDraw.Draw(img)
    d.ellipse((moon[0] - moon_r, moon[1] - moon_r, moon[0] + moon_r, moon[1] + moon_r), fill=MOON + (255,))
    for cx, cy, r in ((-40, -30, 30), (50, 40, 22), (10, 70, 14), (-70, 50, 12)):
        d.ellipse((moon[0] + cx - r, moon[1] + cy - r, moon[0] + cx + r, moon[1] + cy + r), fill=(240, 220, 170, 255))
    return img


def city_silhouette(img, base_y=900, seed=5):
    random.seed(seed)
    layer = Image.new("RGBA", img.size, (0, 0, 0, 0))
    d = ImageDraw.Draw(layer)
    x = 0
    while x < W:
        w = random.randint(110, 240)
        h = random.randint(180, 420)
        d.rectangle((x, base_y - h, x + w, H), fill=(22, 12, 44, 255))
        for wy in range(base_y - h + 30, base_y - 40, 46):
            for wx in range(x + 18, x + w - 30, 40):
                if random.random() < 0.28:
                    d.rectangle((wx, wy, wx + 16, wy + 24), fill=(255, 200, 110, random.randint(150, 230)))
        x += w + random.randint(-10, 20)
    img.alpha_composite(layer)
    # cine en primer plano con marquesina
    d = ImageDraw.Draw(img)
    cx0, cx1, top = 860, 1430, base_y - 380
    d.polygon([(cx0, H), (cx0, top + 60), (cx0 + 60, top), (cx1 - 60, top), (cx1, top + 60), (cx1, H)], fill=(14, 8, 30, 255))
    d.rectangle((cx0 + 50, top + 120, cx1 - 50, top + 210), fill=(40, 16, 30, 255), outline=(255, 190, 90, 255), width=4)
    for bx in range(cx0 + 62, cx1 - 50, 26):
        d.ellipse((bx, top + 128, bx + 9, top + 137), fill=(255, 220, 140, 255))
        d.ellipse((bx, top + 193, bx + 9, top + 202), fill=(255, 220, 140, 255))
    f = fredoka(54, "Bold")
    d.text(((cx0 + cx1) // 2, top + 165), "ETERNAL WAVE", font=f, fill=(255, 210, 120), anchor="mm")
    d.rectangle((cx0 + 230, base_y - 140, cx1 - 230, H), fill=(255, 190, 100, 255))
    d.rectangle((cx0 + 240, base_y - 130, cx1 - 240, H), fill=(60, 26, 20, 255))
    glow = Image.new("RGBA", img.size, (0, 0, 0, 0))
    ImageDraw.Draw(glow).rectangle((cx0 + 40, top + 110, cx1 - 40, top + 220), fill=(255, 170, 70, 120))
    img.alpha_composite(glow.filter(ImageFilter.GaussianBlur(30)))


def fog(img, y0=760, strength=150):
    layer = Image.new("RGBA", img.size, (0, 0, 0, 0))
    d = ImageDraw.Draw(layer)
    random.seed(11)
    for _ in range(40):
        x = random.randint(-200, W)
        y = random.randint(y0, H)
        w = random.randint(300, 700)
        h = random.randint(50, 120)
        d.ellipse((x, y, x + w, y + h), fill=(190, 170, 255, random.randint(30, 70)))
    img.alpha_composite(layer.filter(ImageFilter.GaussianBlur(40)))
    grad = Image.new("RGBA", img.size, (0, 0, 0, 0))
    gd = ImageDraw.Draw(grad)
    for y in range(y0, H):
        a = int(strength * (y - y0) / (H - y0))
        gd.line((0, y, W, y), fill=(30, 18, 60, a))
    img.alpha_composite(grad)


def film_strip(img, y_base=330, amp=60, color=(10, 6, 20, 235), x_start=-50):
    layer = Image.new("RGBA", img.size, (0, 0, 0, 0))
    d = ImageDraw.Draw(layer)
    width = 90
    pts_top, pts_bot = [], []
    for x in range(x_start, W + 60, 20):
        y = y_base + amp * math.sin(x / 260.0)
        pts_top.append((x, y - width / 2))
        pts_bot.append((x, y + width / 2))
    d.polygon(pts_top + pts_bot[::-1], fill=color)
    for x in range(x_start + 10, W + 40, 38):
        y = y_base + amp * math.sin(x / 260.0)
        d.rounded_rectangle((x, y - width / 2 + 8, x + 16, y - width / 2 + 22), 3, fill=(240, 220, 255, 160))
        d.rounded_rectangle((x, y + width / 2 - 22, x + 16, y + width / 2 - 8), 3, fill=(240, 220, 255, 160))
    for x in range(x_start + 80, W, 190):
        y = y_base + amp * math.sin(x / 260.0)
        d.rectangle((x, y - 22, x + 120, y + 22), fill=(120, 90, 200, 80))
    img.alpha_composite(layer)


def portrait(img, path, box, rim=MINT):
    src = Image.open(path).convert("RGBA")
    src = src.crop(src.getbbox())
    bw, bh = box[2] - box[0], box[3] - box[1]
    scale = min(bw / src.width, bh / src.height)
    src = src.resize((int(src.width * scale), int(src.height * scale)), Image.LANCZOS)
    x = box[0] + (bw - src.width) // 2
    y = box[3] - src.height
    alpha = src.split()[3]
    rim_layer = Image.new("RGBA", img.size, (0, 0, 0, 0))
    solid = Image.new("RGBA", src.size, rim + (255,))
    rim_layer.paste(solid, (x, y), alpha)
    img.alpha_composite(rim_layer.filter(ImageFilter.GaussianBlur(14)))
    img.alpha_composite(src, (x, y))


def paw_trail(img, start, end, steps=9, color=MINT + (110,), size=26):
    layer = Image.new("RGBA", img.size, (0, 0, 0, 0))
    d = ImageDraw.Draw(layer)
    for i in range(steps):
        t = i / max(1, steps - 1)
        x = start[0] + (end[0] - start[0]) * t + (14 if i % 2 else -14)
        y = start[1] + (end[1] - start[1]) * t
        paw(d, x, y, size * (0.7 + 0.3 * t), color)
    glow = layer.filter(ImageFilter.GaussianBlur(8))
    img.alpha_composite(glow)
    img.alpha_composite(layer)


def vignette(img, strength=200):
    mask = Image.new("L", img.size, 0)
    d = ImageDraw.Draw(mask)
    d.ellipse((-W * 0.25, -H * 0.35, W * 1.25, H * 1.35), fill=255)
    mask = mask.filter(ImageFilter.GaussianBlur(160))
    dark = Image.new("RGBA", img.size, (6, 2, 14, strength))
    inv = Image.eval(mask, lambda v: 255 - v)
    layer = Image.new("RGBA", img.size, (0, 0, 0, 0))
    layer.paste(dark, (0, 0), inv)
    img.alpha_composite(layer)


def keyart(name, blur=0, darken=0):
    """Ilustración generada con IA (Meshy, Nano Banana Pro) escalada a 1920x1080."""
    path = OUT / name
    if not path.exists():
        return None
    img = Image.open(path).convert("RGBA").resize((W, H), Image.LANCZOS)
    if blur:
        img = img.filter(ImageFilter.GaussianBlur(blur))
    if darken:
        img.alpha_composite(Image.new("RGBA", img.size, (12, 6, 28, darken)))
    return img


# ───────────────────────── imágenes ─────────────────────────

def make_menu_background():
    img = keyart("KeyArt_Menu.png")
    if img is None:
        img = night_background()
        film_strip(img, 360, 50, x_start=820)
        city_silhouette(img)
        fog(img)
    # degradado oscuro a la izquierda para el título y los botones
    panel = Image.new("RGBA", img.size, (0, 0, 0, 0))
    pd = ImageDraw.Draw(panel)
    for x in range(0, 1000):
        a = int(200 * max(0.0, 1.0 - x / 1000.0) ** 1.3)
        pd.line((x, 0, x, H), fill=(10, 4, 26, a))
    img.alpha_composite(panel)
    vignette(img, 150)
    glow_text(img, (110, 60), "The", griffy(80), MOON, glow=(255, 200, 120), glow_radius=10)
    glow_text(img, (100, 130), "Unfinished", griffy(150), MOON, glow=(255, 190, 110))
    glow_text(img, (100, 262), "Cut", griffy(150), (255, 196, 110), glow=(255, 140, 60))
    d = ImageDraw.Draw(img)
    paw(d, 112, 482, 20, MINT + (230,))
    d.text((140, 462), "Ivo + Nilo  ·  una aventura entre películas", font=fredoka(32, "SemiBold"), fill=LILAC,
           stroke_width=3, stroke_fill=INK)
    img.convert("RGB").save(OUT / "T_UI_MenuBG.png")


def make_button(label, selected, name, width=560, height=84):
    img = Image.new("RGBA", (width + 40, height + 40), (0, 0, 0, 0))
    box = (20, 20, 20 + width, 20 + height)
    if selected:
        glow = Image.new("RGBA", img.size, (0, 0, 0, 0))
        ImageDraw.Draw(glow).rounded_rectangle(box, 40, fill=(255, 190, 90, 200))
        img.alpha_composite(glow.filter(ImageFilter.GaussianBlur(12)))
        rounded_panel(img, box, (255, 205, 110, 255), outline=(255, 245, 210, 255), radius=40, width=4, shadow=False)
        color = INK
    else:
        rounded_panel(img, box, (34, 20, 66, 235), outline=(170, 140, 255, 170), radius=40, width=3, shadow=False)
        color = WHITE
    d = ImageDraw.Draw(img)
    d.text((20 + width // 2, 20 + height // 2 + 2), label, font=fredoka(38, "Bold"), fill=color, anchor="mm")
    paw(d, 20 + 44, 20 + height // 2 + 4, 16, (INK + (200,)) if selected else (MINT + (170,)))
    img.save(OUT / f"{name}.png")


def art_crop(region, size):
    """Recorte redondeado de la ilustración del menú (para las tarjetas)."""
    art = keyart("KeyArt_Menu.png")
    if art is None:
        return None
    crop = art.crop(region)
    crop.thumbnail(size, Image.LANCZOS)
    mask = Image.new("L", crop.size, 0)
    ImageDraw.Draw(mask).rounded_rectangle((0, 0, crop.width - 1, crop.height - 1), 22, fill=255)
    crop.putalpha(mask)
    return crop


def card(img, box, title, color, body, font_body, portrait_path=None, icon=None, art=None):
    rounded_panel(img, box, (26, 16, 52, 238), outline=color + (200,), radius=30, width=4)
    d = ImageDraw.Draw(img)
    x0, y0, x1, y1 = box
    text_x = x0 + 34
    if art is not None:
        ay = y0 + (y1 - y0 - art.height) // 2
        img.alpha_composite(art, (x0 + 20, ay))
        ImageDraw.Draw(img).rounded_rectangle((x0 + 20, ay, x0 + 20 + art.width - 1, ay + art.height - 1), 22, outline=color + (255,), width=3)
        text_x = x0 + 40 + art.width
    elif portrait_path:
        portrait(img, portrait_path, (x0 + 14, y0 + 20, x0 + 250, y1 - 16), rim=color)
        text_x = x0 + 270
    if icon:
        icon(d, x0, y0, x1, y1)
        text_x = x0 + 230
    d = ImageDraw.Draw(img)
    d.text((text_x, y0 + 26), title, font=fredoka(40, "Bold"), fill=color)
    draw_wrapped(d, (text_x, y0 + 84), body, font_body, WHITE, x1 - text_x - 30, gap=10)


def icon_marker(d, x0, y0, x1, y1):
    cx, cy = x0 + 115, (y0 + y1) // 2 - 10
    r = 52
    d.polygon([(cx, cy - r), (cx + r, cy), (cx, cy + r), (cx - r, cy)], fill=(255, 205, 110), outline=INK, width=5)
    d.rounded_rectangle((cx - 70, cy + r + 14, cx + 70, cy + r + 56), 14, fill=(10, 6, 20))
    d.text((cx, cy + r + 35), "NILO · 12 m", font=fredoka(24, "Bold"), fill=NILO, anchor="mm")


def icon_eye(d, x0, y0, x1, y1):
    cx, cy = x0 + 115, (y0 + y1) // 2 - 10
    d.polygon([(cx, cy - 30), (cx + 85, cy + 95), (cx - 85, cy + 95)], fill=(120, 30, 60))
    d.polygon([(cx, cy - 30), (cx + 55, cy + 95), (cx - 55, cy + 95)], fill=(190, 45, 70))
    d.ellipse((cx - 40, cy - 90, cx + 40, cy - 20), fill=(255, 240, 230), outline=INK, width=5)
    d.ellipse((cx - 16, cy - 72, cx + 16, cy - 40), fill=RED)
    d.ellipse((cx - 6, cy - 62, cx + 4, cy - 52), fill=INK)


def keycap(d, x, y, label, w=None, color=(250, 240, 255)):
    f = fredoka(28, "Bold")
    tw = d.textlength(label, font=f)
    w = w or max(64, int(tw + 34))
    d.rounded_rectangle((x, y + 6, x + w, y + 66), 14, fill=(8, 4, 18))
    d.rounded_rectangle((x, y, x + w, y + 60), 14, fill=color, outline=INK, width=3)
    d.text((x + w / 2, y + 30), label, font=f, fill=INK, anchor="mm")
    return x + w


def make_howto():
    img = keyart("KeyArt_Menu.png", blur=14, darken=150) or night_background(seed=8, moon=(1760, 120), moon_r=80)
    vignette(img, 160)
    glow_text(img, (W // 2, 70), "¿Cómo se juega?", griffy(96), MOON, glow=(255, 200, 120), anchor="ma")
    d = ImageDraw.Draw(img)
    d.text((W // 2, 190), "Ivo y su gato Nilo quedaron atrapados dentro de las películas de un cine embrujado. Juntos deben cruzar 4 películas y encontrar la salida.",
           font=fredoka(28, "Medium"), fill=LILAC, anchor="ma")
    body = fredoka(29, "Medium")
    card(img, (90, 250, 945, 520), "IVO  ·  el humano", IVO,
         "Juegas en primera persona. Enciende consolas, terminales y proyectores: los objetos que brillan en CELESTE o VIOLETA son suyos.",
         body, portrait_path=OUT / "ivo.png", art=art_crop((1500, 250, 1900, 650), (230, 230)))
    card(img, (975, 250, 1830, 520), "NILO  ·  el gato", NILO,
         "Cámara detrás del gato. Cabe por rejillas y ductos y recoge rollos de película: los objetos que brillan en VERDE son suyos.",
         body, portrait_path=OUT / "nilo.png", art=art_crop((1300, 740, 1560, 1000), (230, 230)))
    card(img, (90, 550, 945, 820), "Sigue el rombo", (255, 205, 110),
         "Arriba a la izquierda verás el OBJETIVO, dónde está y quién lo hace. Un rombo marca el lugar con la distancia. Acércate y pulsa E.",
         body, icon=icon_marker)
    card(img, (975, 550, 1830, 820), "Evita la luz roja", RED,
         "Los vigilantes tienen un cono de luz roja. Si te ven mucho rato, la película se rebobina al último punto de control.",
         body, icon=icon_eye)
    rounded_panel(img, (90, 850, 1830, 960), (16, 10, 34, 230), outline=(170, 140, 255, 140), radius=26, width=3)
    d = ImageDraw.Draw(img)
    x, y = 120, 875
    small = fredoka(26, "SemiBold")
    for keys, label in ((("W", "A", "S", "D"), "moverse"), (("RATÓN",), "mirar"), (("E",), "usar"), (("TAB",), "cambiar Ivo / Nilo"), (("F",), "seguir / esperar"), (("P",), "pausa")):
        for k in keys:
            x = keycap(d, x, y, k) + 6
        d.text((x + 8, y + 30), label, font=small, fill=WHITE, anchor="lm")
        x += int(d.textlength(label, font=small)) + 46
    img.convert("RGB").save(OUT / "T_UI_HowTo.png")


def make_prologue():
    img = keyart("KeyArt_Alt.png", blur=6, darken=150) or night_background(seed=21, moon=(960, 230), moon_r=120)
    vignette(img, 220)
    glow_text(img, (W // 2, 90), "Corte 001 — final registrado", griffy(70), (255, 196, 110), glow=(255, 140, 60), anchor="ma")
    d = ImageDraw.Draw(img)
    y = 420
    for line in ("Un proyector suena en una sala que nadie recuerda.",
                 "Ivo cruza una puerta. Mira atrás. Espera.",
                 "En su mano, una campanilla. Nilo no aparece.",
                 "La cinta se rebobina..."):
        d.text((W // 2, y), line, font=fredoka(40, "Medium"), fill=WHITE, anchor="ma", stroke_width=3, stroke_fill=INK)
        y += 62
    d.text((W // 2, y + 24), "Ayuda a Ivo y a Nilo a escapar juntos de las películas.", font=fredoka(32, "SemiBold"), fill=MINT, anchor="ma",
           stroke_width=3, stroke_fill=INK)
    paw_trail(img, (300, 860), (1620, 840), steps=14, size=20)
    img.convert("RGB").save(OUT / "T_UI_Prologue.png")


def make_pause_title():
    img = Image.new("RGBA", (900, 220), (0, 0, 0, 0))
    glow_text(img, (450, 20), "Pausa", griffy(140), MOON, glow=(255, 200, 120), anchor="ma")
    img.save(OUT / "T_UI_PauseTitle.png")


BUTTONS = {
    "Continue": "CONTINUAR",
    "NewGame": "NUEVA PARTIDA",
    "HowTo": "CÓMO JUGAR",
    "Quit": "SALIR",
    "Begin": "¡EMPEZAR!",
    "Back": "VOLVER",
    "Resume": "SEGUIR JUGANDO",
    "Restart": "EMPEZAR DE NUEVO",
    "MainMenu": "MENÚ PRINCIPAL",
    "QuitGame": "SALIR DEL JUEGO",
}


# ───────────────────────── fuente del HUD (atlas de glifos) ─────────────────────────
HUD_CHARS = ("".join(chr(c) for c in range(32, 127)) + "áéíóúÁÉÍÓÚñÑüÜ¿¡·—–…“”‘’«»ºª°×")


def make_font_atlas(size=72, atlas=1024, pad=8):
    """Atlas blanco con alfa + tabla de glifos en C++ (TUCFontData.h). El HUD dibuja el texto con esto,
    así la fuente de juego se ve igual en cualquier versión de Unreal."""
    font = ImageFont.truetype(str(FONTS / "LilitaOne-Regular.ttf"), size)
    ascent, descent = font.getmetrics()
    img = Image.new("RGBA", (atlas, atlas), (255, 255, 255, 0))
    d = ImageDraw.Draw(img)
    x = y = pad
    row_h = 0
    glyphs = []
    for ch in HUD_CHARS:
        adv = font.getlength(ch)
        if ch == " ":
            glyphs.append((ord(ch), 0, 0, 0, 0, 0, 0, adv))
            continue
        box = font.getbbox(ch, anchor="ls")
        if box[2] - box[0] <= 0 or not font.getmask(ch).getbbox():
            continue
        w = box[2] - box[0] + pad * 2
        h = box[3] - box[1] + pad * 2
        if x + w > atlas:
            x = pad
            y += row_h + pad
            row_h = 0
        if y + h > atlas:
            raise RuntimeError("atlas lleno")
        d.text((x + pad - box[0], y + pad - box[1]), ch, font=font, fill=(255, 255, 255, 255), anchor="ls")
        glyphs.append((ord(ch), x, y, w, h, box[0] - pad, box[1] - pad, adv))
        x += w + pad
        row_h = max(row_h, h)
    img.save(OUT / "T_UI_FontAtlas.png")

    lines = [
        "\ufeff// Generado por Tools/UI/make_ui.py — no editar a mano.",
        "// Fuente del HUD: Lilita One (Google Fonts, SIL Open Font License 1.1).",
        "#pragma once",
        "",
        "#include \"CoreMinimal.h\"",
        "",
        "struct FTUCGlyph",
        "{",
        "    uint32 Code;",
        "    uint16 X, Y, W, H;",
        "    int16 OffX, OffY;",
        "    float Advance;",
        "};",
        "",
        f"constexpr float GTUCFontAscent = {ascent}.0f;",
        f"constexpr float GTUCFontDescent = {descent}.0f;",
        f"constexpr float GTUCFontAtlasSize = {atlas}.0f;",
        "",
        "static const FTUCGlyph GTUCGlyphs[] =",
        "{",
    ]
    for code, gx, gy, gw, gh, ox, oy, adv in glyphs:
        lines.append(f"    {{ {code}u, {gx}, {gy}, {gw}, {gh}, {ox}, {oy}, {adv:.2f}f }},")
    lines += ["};", ""]
    (ROOT / "Source" / "TheUnfinishedCut" / "TUCFontData.h").write_bytes("\n".join(lines).encode("utf-8"))
    print(f"atlas: {len(glyphs)} glifos, alto de línea {ascent + descent}px")


if __name__ == "__main__":
    OUT.mkdir(parents=True, exist_ok=True)
    make_menu_background()
    make_howto()
    make_prologue()
    make_pause_title()
    make_font_atlas()
    for key, label in BUTTONS.items():
        make_button(label, False, f"T_UI_Btn_{key}_0")
        make_button(label, True, f"T_UI_Btn_{key}_1")
    (OUT / "ui.json").write_text(json.dumps({"buttons": BUTTONS}, ensure_ascii=False, indent=1), encoding="utf-8")
    print("UI lista en", OUT)
