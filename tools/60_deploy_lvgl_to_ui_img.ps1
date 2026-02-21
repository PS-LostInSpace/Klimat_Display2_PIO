# tools/60_deploy_lvgl_to_ui_img.ps1
param(
  [int]$Size = 128
)

$ErrorActionPreference = "Stop"

$src = "assets/icons/out/lvgl/$Size"
$dst = "lib/ui/img"

New-Item -ItemType Directory -Force -Path $dst | Out-Null
Copy-Item "$src\kd_weather_*_$Size.c" $dst -Force

Write-Host "OK: deployed size $Size icons -> $dst"