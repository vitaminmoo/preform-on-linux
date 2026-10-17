#!/usr/bin/env python3
"""Make Lutris art for PreForm from the icon in the user's own PreFormLauncher.exe.

Usage: preform-art.py <path to PreFormLauncher.exe> <game slug>
       preform-art.py --icon <path to PreFormLauncher.exe> <out.png>

The --icon form just saves the largest icon as a PNG (used by install.sh); it needs
Pillow only if the exe's icon isn't PNG-compressed, which PreForm's is.

Writes the icon, banner and cover art Lutris looks for by slug, skipping any that
already exist. Lutris.net has no entry for PreForm to download art from, and nothing
from Formlabs is redistributed this way. Never fails the install: on any error it
prints why and exits 0.
"""
import io
import os
import struct
import sys

# PreForm's icon tile runs from about (25, 117, 207) at the top to (21, 99, 176)
BLUE_TOP = (25, 117, 207)
BLUE_BOTTOM = (16, 70, 130)
RT_ICON, RT_GROUP_ICON = 3, 14
PNG_MAGIC = b"\x89PNG\r\n\x1a\n"


def read_icon_bytes(exe):
    """Return the largest icon in a PE file's resources as PNG or .ico bytes (no pefile needed)."""
    data = open(exe, "rb").read()
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    nsect, optsize = struct.unpack_from("<H12xH", data, pe + 6)
    opt = pe + 24
    rsrc_rva = struct.unpack_from("<I", data, opt + (112 if data[opt] == 0x0B and data[opt + 1] == 2 else 96) + 2 * 8)[0]
    for i in range(nsect):
        vsize, va, size, raw = struct.unpack_from("<IIII", data, opt + optsize + 40 * i + 8)
        if va <= rsrc_rva < va + max(vsize, size):
            base = raw + rsrc_rva - va
            delta = va - raw  # RVA - file offset inside .rsrc
            break
    else:
        raise ValueError("no resource section")

    def entries(off):
        named, ids = struct.unpack_from("<12xHH", data, base + off)
        for i in range(named + ids):
            name, target = struct.unpack_from("<II", data, base + off + 16 + 8 * i)
            yield name, target

    def leaf(target):
        while target & 0x80000000:  # descend to the first language
            target = next(entries(target & 0x7FFFFFFF))[1]
        rva, size = struct.unpack_from("<II", data, base + target)
        return data[rva - delta:rva - delta + size]

    def by_type(t):
        for name, target in entries(0):
            if name == t:
                return {n: tgt for n, tgt in entries(target & 0x7FFFFFFF)}
        return {}

    icons = by_type(RT_ICON)
    group = leaf(next(iter(by_type(RT_GROUP_ICON).values())))
    count = struct.unpack_from("<H", group, 4)[0]
    best = max((struct.unpack_from("<BBBBHHIH", group, 6 + 14 * i) for i in range(count)),
               key=lambda e: ((e[0] or 256) * (e[1] or 256), e[5]))
    img = leaf(icons[best[7]])
    if img[:8] != PNG_MAGIC:  # a bare DIB: wrap it in a one-image .ico
        img = struct.pack("<HHHBBBBHHII", 0, 1, 1, *best[:4], best[4], best[5], len(img), 22) + img
    return img


def read_icon(exe):
    """Return the largest icon in a PE file's resources as a PIL image."""
    from PIL import Image

    return Image.open(io.BytesIO(read_icon_bytes(exe))).convert("RGBA")


def save_icon(exe, out):
    img = read_icon_bytes(exe)
    os.makedirs(os.path.dirname(os.path.abspath(out)), exist_ok=True)
    if img[:8] == PNG_MAGIC:
        with open(out, "wb") as f:
            f.write(img)
    else:
        read_icon(exe).save(out)
    print(f"wrote {out}")


def gradient(size):
    from PIL import Image

    w, h = size
    img = Image.new("RGB", size)
    px = img.load()
    for y in range(h):
        t = y / max(h - 1, 1)
        row = tuple(round(a + (b - a) * t) for a, b in zip(BLUE_TOP, BLUE_BOTTOM))
        for x in range(w):
            px[x, y] = row
    return img


def font(size):
    from PIL import ImageFont

    for name in ("DejaVuSans-Bold.ttf", "LiberationSans-Bold.ttf", "NotoSans-Bold.ttf"):
        try:
            return ImageFont.truetype(name, size)
        except OSError:
            pass
    return ImageFont.load_default(size=size)


def place(canvas, icon, side, xy):
    from PIL import Image

    canvas.paste(icon.resize((side, side), Image.LANCZOS), xy, icon.resize((side, side), Image.LANCZOS))


def cover(icon, size=(264, 352)):
    from PIL import ImageDraw

    img = gradient(size)
    place(img, icon, 168, ((size[0] - 168) // 2, 70))
    draw = ImageDraw.Draw(img)
    f = font(36)
    w = draw.textlength("PreForm", font=f)
    draw.text(((size[0] - w) / 2, 262), "PreForm", font=f, fill="white")
    return img


def banner(icon, size=(184, 69)):
    from PIL import ImageDraw

    img = gradient(size)
    place(img, icon, 53, (8, 8))
    ImageDraw.Draw(img).text((70, 20), "PreForm", font=font(23), fill="white")
    return img


def main():
    if sys.argv[1] == "--icon":
        save_icon(sys.argv[2], sys.argv[3])
        return
    exe, slug = sys.argv[1], sys.argv[2]
    try:
        from lutris.settings import BANNER_PATH, COVERART_PATH, ICON_PATH
    except ImportError:  # same defaults Lutris uses
        data = os.environ.get("XDG_DATA_HOME") or os.path.expanduser("~/.local/share")
        ICON_PATH = os.path.join(data, "icons", "hicolor", "128x128", "apps")
        BANNER_PATH = os.path.join(data, "lutris", "banners")
        COVERART_PATH = os.path.join(data, "lutris", "coverart")

    icon = read_icon(exe)
    outputs = {
        os.path.join(ICON_PATH, f"lutris_{slug}.png"): lambda: icon.resize((128, 128)),
        os.path.join(BANNER_PATH, f"{slug}.jpg"): lambda: banner(icon),
        os.path.join(COVERART_PATH, f"{slug}.jpg"): lambda: cover(icon),
    }
    for path, make in outputs.items():
        if os.path.exists(path):
            print(f"keeping existing {path}")
            continue
        os.makedirs(os.path.dirname(path), exist_ok=True)
        make().save(path)
        print(f"wrote {path}")


if __name__ == "__main__":
    try:
        main()
    except Exception as ex:  # art is cosmetic; never break the install over it
        print(f"preform-art: skipped ({ex!r})")
    sys.exit(0)
