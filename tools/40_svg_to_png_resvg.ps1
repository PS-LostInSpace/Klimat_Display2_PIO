# tools/40_svg_to_png_resvg.ps1
param(
  [string]$InDir = "assets/icons/work/svg_final",
  [int[]]$Sizes = @(128,169,192)
)

$ErrorActionPreference = "Stop"

# Ensure resvg exists
$resvg = Get-Command resvg -ErrorAction SilentlyContinue
if (-not $resvg) {
  throw "resvg not found. Install with: py -m pip install --user resvg-cli  (then restart terminal)"
}

foreach ($s in $Sizes) {
  $outDir = "assets/icons/out/png/$s"
  New-Item -ItemType Directory -Force -Path $outDir | Out-Null

  Get-ChildItem $InDir -Filter *.svg | ForEach-Object {
    $out = Join-Path $outDir ($_.BaseName + ".png")
    resvg --width $s --height $s $_.FullName $out
  }

  Write-Host "OK: rendered PNG size $s -> $outDir"
}