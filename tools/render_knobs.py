# Renders knob filmstrips: python3 tools/render_knobs.py <lobed|silver|dark|footswitch> 65 160 (footswitch: 1 frame) → knob-<style>.png in cwd.
import numpy as np, sys
from scipy.ndimage import gaussian_filter, shift as nshift
from PIL import Image

SS = 2                      # supersample
OUT = int(sys.argv[3]) if len(sys.argv) > 3 else 160
N = 2 * OUT                 # render size
c = (np.arange(N) + 0.5) / N * 2 - 1
X, Y = np.meshgrid(c, c)
R = np.hypot(X, Y)
TH = np.arctan2(Y, X)
L = np.array([-0.42, -0.62, 0.66]); L /= np.linalg.norm(L)
V = np.array([0.0, 0.0, 1.0])
H = L + V; H /= np.linalg.norm(H)
SCALE = 1.0 / 0.82          # knob footprint radius 0.82 of half-size leaves room for shadow

def smooth(a, b, x):
    t = np.clip((x - a) / (b - a), 0, 1); return t * t * (3 - 2 * t)

def normals(h, k):
    gy, gx = np.gradient(h * k)
    n = np.dstack([-gx, -gy, np.ones_like(h)])
    return n / np.linalg.norm(n, axis=2, keepdims=True)

def env(rv):
    # soft studio: big softbox upper-left, dim floor, faint rim from right
    sb = np.array([-0.35, -0.55, 0.76]); sb /= np.linalg.norm(sb)
    d = np.clip(rv @ sb, -1, 1)
    box = smooth(0.80, 0.97, d) * 1.6
    rim = smooth(0.85, 0.99, rv @ (np.array([0.7, 0.2, 0.68]) / np.linalg.norm([0.7, 0.2, 0.68]))) * 0.35
    sky = 0.10 + 0.12 * np.clip(-rv[..., 1], 0, 1)
    return box + rim + sky

def shade(h, albedo, metal, rough_exp, aniso=False, k=None, emissive=None, fill=0.0):
    n = normals(h, k)
    ndl = np.clip(n @ L, 0, 1)
    ndv = np.clip(n[..., 2], 0, 1)
    rv = 2 * ndv[..., None] * n - V
    e = env(rv)[..., None]
    if aniso:
        t = np.dstack([-np.sin(TH), np.cos(TH), np.zeros_like(TH)])
        th = np.clip(np.abs(t @ H), 0, 1)
        spec = (np.sqrt(1 - th ** 2) ** rough_exp)[..., None] * 0.9
    else:
        spec = (np.clip(n @ H, 0, 1) ** rough_exp)[..., None]
    if metal:
        f = albedo * (0.25 * ndl[..., None] + 0.08) + albedo * e * 0.55 + spec * albedo * 1.2
    else:
        fres = (0.04 + 0.96 * (1 - ndv) ** 5)[..., None]
        f = albedo * (0.85 * ndl[..., None] + 0.18) + (fres * 0.9 + 0.07) * e * 0.75 + spec * 0.6
    ao = 1 - np.clip((gaussian_filter(h, N * 0.02) - h) * 2.2, 0, 0.55)
    f = f * ao[..., None]
    f = f + albedo * fill * ao[..., None]
    if emissive is not None: f = f + emissive
    return f

def alu(a, base, ptr_rgb, glow):
    t = TH - a
    h = np.zeros_like(R)
    body = R < 0.80
    knurl = (R >= 0.70) & body
    h[body] = 0.85
    h[knurl] = 0.85 - 0.40 * smooth(0.70, 0.80, R[knurl]) + 0.035 * np.cos(64 * t[knurl])
    face = R < 0.70
    h[face] = 0.88 + 0.0016 * np.sin(R[face] * 420) - 0.02 * smooth(0.66, 0.70, R[face])
    dimple = R < 0.12
    h[dimple] -= 0.025 * (1 - (R[dimple] / 0.12) ** 2)
    alb = np.empty(R.shape + (3,)); alb[:] = base
    ux, uy = np.cos(a - np.pi / 2), np.sin(a - np.pi / 2)
    along = X * ux + Y * uy; perp = np.abs(-X * uy + Y * ux)
    groove = (perp < 0.028) & (along > 0.30) & (along < 0.64)
    h[groove] -= 0.03
    em = np.zeros(R.shape + (3,))
    if glow:
        em[groove] = ptr_rgb
        halo = gaussian_filter(groove.astype(float), N * 0.012)[..., None] * np.array(ptr_rgb) * 0.8
        em += halo * body[..., None]
    else:
        alb[groove] = ptr_rgb
    col = shade(h, alb, True, 26, aniso=True, k=N * 0.12, emissive=em)
    return col, body


def lobed(a):
    t = TH - a
    re = 0.76 + 0.03 * np.cos(7 * t)
    h = np.zeros_like(R)
    body = R < re
    u = R[body] / re[body]
    h[body] = 0.25 + 0.67 * np.clip(1 - smooth(0.62, 1.0, u) ** 1.4, 0, 1) + 0.04 * (1 - u ** 2)
    alb = np.full(R.shape + (3,), 0.045)
    ux, uy = np.cos(a - np.pi / 2), np.sin(a - np.pi / 2)
    along = X * ux + Y * uy; perp = np.abs(-X * uy + Y * ux)
    alb[(perp < 0.032) & (along > 0.08) & (along < 0.58)] = [0.93, 0.92, 0.89]
    return shade(h, alb, False, 46, k=N * 0.22), body

def footswitch(a):
    ap = 0.80 * np.cos(np.pi / 6)
    d = np.max([np.abs(X * np.cos(k * np.pi / 3) + Y * np.sin(k * np.pi / 3)) for k in range(3)], axis=0)
    nut = d < ap
    h = np.zeros_like(R)
    h[nut] = 0.34 - 0.20 * smooth(ap - 0.07, ap, d[nut])
    ring = R < 0.58
    h[ring] = 0.42
    cap = R < 0.52
    h[cap] = 0.95 + 0.06 * (1 - (R[cap] / 0.52) ** 2) - 0.30 * smooth(0.45, 0.52, R[cap]) ** 2
    alb = np.empty(R.shape + (3,)); alb[:] = [0.80, 0.81, 0.83]
    alb[nut & ~ring] = [0.66, 0.67, 0.69]
    return shade(h, alb, True, 22, aniso=True, k=N * 0.18, fill=0.16), nut | ring

STYLES = {
    "lobed": lambda a: lobed(a),
    "footswitch": lambda a: footswitch(a),
    "silver": lambda a: alu(a, np.array([0.80, 0.80, 0.82]), [0.12, 0.12, 0.13], False),
    "dark": lambda a: alu(a, np.array([0.27, 0.27, 0.29]), [0.31, 0.66, 0.94], True),
}

def frame(style, a):
    col, mask = STYLES[style](a)
    m = gaussian_filter(mask.astype(float), 0.6)
    sh = gaussian_filter(nshift(mask.astype(float), (N * 0.055, N * 0.035), order=1), N * 0.035) * 0.55
    sh += gaussian_filter(mask.astype(float), N * 0.012) * 0.35
    sh = np.clip(sh, 0, 0.8)
    col = np.clip(col, 0, 1) ** (1 / 1.15)
    alpha = m + sh * (1 - m)
    rgb = col * (m / np.maximum(alpha, 1e-6))[..., None]
    img = np.dstack([rgb, alpha])
    img = img.reshape(OUT, SS, OUT, SS, 4).mean(axis=(1, 3))
    return Image.fromarray((np.clip(img, 0, 1) * 255).astype(np.uint8), "RGBA")

style, frames = sys.argv[1], int(sys.argv[2])
angles = np.radians(np.linspace(-135, 135, frames)) if frames > 1 else [0.0]
strip = Image.new("RGBA", (OUT, OUT * frames))
for i, a in enumerate(angles):
    strip.paste(frame(style, a), (0, i * OUT))
strip.save(f"knob-{style}.png", optimize=True)
