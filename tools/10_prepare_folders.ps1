# tools/10_prepare_folders.ps1
$ErrorActionPreference = "Stop"

$dirs = @(
  "assets/icons/sources/lxg_weather_icons_3.0.1",
  "assets/icons/work/svg_trim",
  "assets/icons/work/svg_trim_backup",
  "assets/icons/work/svg_outline",
  "assets/icons/work/svg_final",
  "assets/icons/out/png/128",
  "assets/icons/out/png/169",
  "assets/icons/out/png/192",
  "assets/icons/out/lvgl/128",
  "assets/icons/out/lvgl/169",
  "assets/icons/out/lvgl/192"
)

foreach ($d in $dirs) { New-Item -ItemType Directory -Force -Path $d | Out-Null }
Write-Host "OK: folders prepared."