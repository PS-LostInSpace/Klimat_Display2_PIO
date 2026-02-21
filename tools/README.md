# KD2 Weather Icon Tooling (Windows 11 / PowerShell)

This folder contains a reproducible asset pipeline for generating
LVGL weather icons for KD2 (and future KD1/KD3).

The pipeline is designed for:
- Windows 11
- Limited admin rights (work laptop)
- PowerShell
- VS Code
- No Cairo dependencies

It avoids Cairo-based tools (`cairosvg`, `reportlab renderPM`) because
they require `cairo-2.dll`, which is typically unavailable in locked-down environments.

PowerShell note:
In `.ps1` scripts, `param(...)` must be the first executable statement.
If anything (for example `$ErrorActionPreference = "Stop"`) appears before `param`,
you may get parser errors such as `InvalidLeftHandSide`.

Instead we use:

SVG → resvg (PNG) → Pillow → LVGL Indexed 2-bit C file


------------------------------------------------------------
FOLDER LAYOUT
------------------------------------------------------------

All icon assets live under:

assets/icons/

Sources (never modify)
assets/icons/sources/lxg_weather_icons_3.0.1/

Original SVG files downloaded from LXG weather-icons.


Work (editable SVG pipeline)
assets/icons/work/
  svg_trim/
  svg_outline/
  svg_final/
  svg_trim_backup/

Pipeline stages:
1. svg_trim     → stroke width + metadata cleanup
2. svg_outline  → enforce outline style
3. svg_final    → clipPath cleanup (safe for rasterization)


Outputs
assets/icons/out/
  png/
    128/
    169/
    192/
  lvgl/
    128/
    169/
    192/

Final firmware-ready icons are deployed to:

lib/ui/img/


------------------------------------------------------------
SCRIPTS
------------------------------------------------------------

10_prepare_folders.ps1
Creates the full folder structure under assets/icons/.
Run once at project setup.


20_fetch_svgs_lxg.ps1
Downloads the 10 base weather icons (LXG Weather Icons v3.0.1).

Output:
assets/icons/sources/lxg_weather_icons_3.0.1/


30_trim_pipeline.ps1
Builds cleaned outline SVGs in a reproducible way.

Pipeline:
sources → work/svg_trim → work/svg_outline → work/svg_final

Parameters:
- -SourceDir
- -WorkRoot
- -StrokeWidth (1.8 / 2.2 / 2.4 recommended)
- -ForceFillNone (default: true)

Example:

.\tools\30_trim_pipeline.ps1 -StrokeWidth 2.2

This script internally calls:
- trim_svgs.ps1
- outlineify_svgs.ps1
- fix_clipPath.ps1

All three are parameterized (v2) and can also be run independently.


40_svg_to_png_resvg.ps1
Rasterizes SVG → PNG using resvg.

Sizes default to:
128, 169, 192

Example:

.\tools\40_svg_to_png_resvg.ps1

Requires:
pip install --user resvg-cli


50_png_to_lvgl_c.ps1
Converts PNG → LVGL Indexed 2-bit C files using:

tools/png_to_lvgl_i2.py

Output:
assets/icons/out/lvgl/<size>/

Example:

.\tools\50_png_to_lvgl_c.ps1 -Size 192


60_deploy_lvgl_to_ui_img.ps1
Copies generated LVGL .c files into firmware directory:

lib/ui/img/

Example:

.\tools\60_deploy_lvgl_to_ui_img.ps1 -Size 192


------------------------------------------------------------
TYPICAL WORKFLOW
------------------------------------------------------------

.\tools\10_prepare_folders.ps1
.\tools\20_fetch_svgs_lxg.ps1 ( if it fails use .\tools\20_fetch_svgs_lxg_curl.ps1)
.\tools\30_trim_pipeline.ps1 -StrokeWidth 2.2
.\tools\40_svg_to_png_resvg.ps1
.\tools\50_png_to_lvgl_c.ps1 -Size 192
.\tools\60_deploy_lvgl_to_ui_img.ps1 -Size 192


------------------------------------------------------------
LVGL FORMAT
------------------------------------------------------------

Generated icons use:

LV_IMG_CF_INDEXED_2BIT

Palette:
- Index 0 = Transparent
- Index 3 = Black (forced for strong e-ink contrast)

This keeps:
- Flash usage low
- High readability on e-paper
- Deterministic rendering


------------------------------------------------------------
DEPRECATED
------------------------------------------------------------

svg_to_lvgl_i2.py exists but is NOT recommended on Windows work machines
because it requires Cairo (cairo-2.dll).

Use instead:
resvg → PNG → png_to_lvgl_i2.py


------------------------------------------------------------
DESIGN NOTES
------------------------------------------------------------

- Outline style optimized for e-ink
- Stroke width is the primary tuning parameter
- 192px is recommended default size for KD2
- All steps are reproducible and script-based (no manual terminal hacks)

For icon-only visual comparison (128/169/192) using compile-time UI test mode,
see section "UI-WX_ICON test" in the project root README.

End of document.