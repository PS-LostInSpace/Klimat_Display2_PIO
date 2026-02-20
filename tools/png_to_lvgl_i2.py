# tools/png_to_lvgl_i2.py
# PNG -> LVGL C image (LV_IMG_CF_INDEXED_2BIT)

import sys
from pathlib import Path
from PIL import Image

PALETTE_ARGB8888 = [
    0xFFFFFFFF,  # white
    0xFFAAAAAA,  # light gray
    0xFF555555,  # dark gray
    0xFF000000,  # black
]

def luminance(r, g, b):
    return int(0.2126*r + 0.7152*g + 0.0722*b)

def to_idx(r, g, b, a):
    if a < 128:
        return 0
    y = luminance(r, g, b)
    if y > 210: return 0
    if y > 140: return 1
    if y > 70:  return 2
    return 3

def pack(indices, w, h):
    out = bytearray()
    for y in range(h):
        row = indices[y*w:(y+1)*w]
        for x in range(0, w, 4):
            p0 = row[x]   if x+0 < w else 0
            p1 = row[x+1] if x+1 < w else 0
            p2 = row[x+2] if x+2 < w else 0
            p3 = row[x+3] if x+3 < w else 0
            out.append(((p0&3)<<6) | ((p1&3)<<4) | ((p2&3)<<2) | (p3&3))
    return bytes(out)

def main():
    if len(sys.argv) != 4:
        print("Usage: python tools/png_to_lvgl_i2.py input.png output.c symbol")
        sys.exit(2)

    in_path = Path(sys.argv[1])
    out_path = Path(sys.argv[2])
    sym = sys.argv[3]

    img = Image.open(in_path).convert("RGBA")
    w, h = img.size
    px = list(img.getdata())

    idx = [to_idx(r,g,b,a) for (r,g,b,a) in px]
    packed = pack(idx, w, h)

    palette_bytes = b"".join(int(v).to_bytes(4, "little") for v in PALETTE_ARGB8888)
    blob = palette_bytes + packed

    map_name = f"{sym}_map"

    out_path.parent.mkdir(parents=True, exist_ok=True)
    with open(out_path, "w", encoding="utf-8") as f:
        f.write("// Auto-generated from PNG\n")
        f.write("#include <lvgl.h>\n\n")
        f.write(f"static const uint8_t {map_name}[] = {{\n")
        for i in range(0, len(blob), 16):
            chunk = blob[i:i+16]
            f.write("  " + ", ".join(f"0x{b:02X}" for b in chunk) + ",\n")
        f.write("};\n\n")
        f.write(f"const lv_img_dsc_t {sym} = {{\n")
        f.write("  .header = {.cf = LV_IMG_CF_INDEXED_2BIT, ")
        f.write(f".w = {w}, .h = {h}}},\n")
        f.write(f"  .data_size = {len(blob)},\n")
        f.write(f"  .data = {map_name},\n")
        f.write("};\n")

    print(f"Wrote {out_path}")

if __name__ == "__main__":
    main()