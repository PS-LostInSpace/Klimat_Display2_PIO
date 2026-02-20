# tools/svg_to_lvgl_i2.py
# Convert SVG -> raster (128x128) using svglib/reportlab -> LVGL Indexed 2-bit C file

import sys
from pathlib import Path
from io import BytesIO
from PIL import Image
from svglib.svglib import svg2rlg
from reportlab.graphics import renderPM

PALETTE = [
    0xFFFFFFFF,
    0xFFAAAAAA,
    0xFF555555,
    0xFF000000,
]

def luminance(r, g, b):
    return int(0.2126*r + 0.7152*g + 0.0722*b)

def to_index(r, g, b, a):
    if a < 128:
        return 0
    y = luminance(r, g, b)
    if y > 210:
        return 0
    elif y > 140:
        return 1
    elif y > 70:
        return 2
    else:
        return 3

def pack(indices, w, h):
    out = bytearray()
    for y in range(h):
        row = indices[y*w:(y+1)*w]
        for x in range(0, w, 4):
            p0 = row[x] if x+0 < w else 0
            p1 = row[x+1] if x+1 < w else 0
            p2 = row[x+2] if x+2 < w else 0
            p3 = row[x+3] if x+3 < w else 0
            out.append(((p0&3)<<6) | ((p1&3)<<4) | ((p2&3)<<2) | (p3&3))
    return bytes(out)

def render_svg_to_pil(svg_path, size):
    drawing = svg2rlg(str(svg_path))
    sx = size / float(drawing.width)
    sy = size / float(drawing.height)
    s = min(sx, sy)
    drawing.scale(s, s)
    png_bytes = renderPM.drawToString(drawing, fmt="PNG")
    img = Image.open(BytesIO(png_bytes)).convert("RGBA")
    img = img.resize((size, size), Image.LANCZOS)
    return img

def main():
    if len(sys.argv) < 4:
        print("Usage: python svg_to_lvgl_i2.py input.svg output.c symbol_name [size]")
        sys.exit(2)

    in_svg = Path(sys.argv[1])
    out_c  = Path(sys.argv[2])
    symbol = sys.argv[3]
    size = int(sys.argv[4]) if len(sys.argv) >= 5 else 128

    img = render_svg_to_pil(in_svg, size)
    w, h = img.size
    px = list(img.getdata())

    indices = [to_index(r, g, b, a) for (r, g, b, a) in px]
    packed = pack(indices, w, h)

    palette_bytes = b"".join(v.to_bytes(4, "little") for v in PALETTE)
    blob = palette_bytes + packed

    map_name = symbol + "_map"

    out_c.parent.mkdir(parents=True, exist_ok=True)
    with open(out_c, "w", encoding="utf-8") as f:
        f.write("// Auto-generated from SVG\n")
        f.write("#include <lvgl.h>\n\n")
        f.write(f"static const uint8_t {map_name}[] = {{\n")
        for i in range(0, len(blob), 16):
            chunk = blob[i:i+16]
            f.write("  " + ", ".join(f"0x{b:02X}" for b in chunk) + ",\n")
        f.write("};\n\n")
        f.write(f"const lv_img_dsc_t {symbol} = {{\n")
        f.write("  .header = {\n")
        f.write("    .cf = LV_IMG_CF_INDEXED_2BIT,\n")
        f.write(f"    .w = {w},\n")
        f.write(f"    .h = {h},\n")
        f.write("  },\n")
        f.write(f"  .data_size = {len(blob)},\n")
        f.write(f"  .data = {map_name},\n")
        f.write("};\n")

    print(f"Created {out_c}")

if __name__ == "__main__":
    main()