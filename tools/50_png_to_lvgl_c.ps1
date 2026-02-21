# tools/50_png_to_lvgl_c.ps1
param(
  [int]$Size = 128,
  [string]$PngDir = "",
  [string]$OutDir = ""
)

$ErrorActionPreference = "Stop"

if ($PngDir -eq "") { $PngDir = "assets/icons/out/png/$Size" }
if ($OutDir -eq "") { $OutDir = "assets/icons/out/lvgl/$Size" }

New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

# Pillow dependency check happens when script runs
Get-ChildItem $PngDir -Filter *.png | ForEach-Object {
  $name = "kd_weather_" + $_.BaseName.Replace("-", "_") + "_$Size"
  $out  = Join-Path $OutDir ($name + ".c")
  python tools/png_to_lvgl_i2.py $_.FullName $out $name
}

Write-Host "OK: LVGL C generated -> $OutDir"