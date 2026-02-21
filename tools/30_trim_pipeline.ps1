# tools/30_trim_pipeline.ps1 (v3)
# Build cleaned outline SVGs for e-ink:
# sources -> work/svg_trim -> work/svg_outline -> work/svg_final
#
# Requires v2 scripts:
# - tools/trim_svgs.ps1
# - tools/outlineify_svgs.ps1
# - tools/fix_clipPath.ps1

param(
  [string]$SourceDir = "assets/icons/sources/lxg_weather_icons_3.0.1",
  [string]$WorkRoot  = "assets/icons/work",
  [string]$StrokeWidth = "2.2",
  [bool]$ForceFillNone = $true
)

$ErrorActionPreference = "Stop"

$trimDir     = Join-Path $WorkRoot "svg_trim"
$outlineDir  = Join-Path $WorkRoot "svg_outline"
$finalDir    = Join-Path $WorkRoot "svg_final"
$trimBackup  = Join-Path $WorkRoot "svg_trim_backup"

# Create folders
New-Item -ItemType Directory -Force -Path $trimDir, $outlineDir, $finalDir, $trimBackup | Out-Null

# Step 0: start from clean work dirs
Remove-Item -Path (Join-Path $trimDir "*.svg")    -Force -ErrorAction SilentlyContinue
Remove-Item -Path (Join-Path $outlineDir "*.svg") -Force -ErrorAction SilentlyContinue
Remove-Item -Path (Join-Path $finalDir "*.svg")   -Force -ErrorAction SilentlyContinue

# Step A: copy sources -> svg_trim
Copy-Item -Path (Join-Path $SourceDir "*.svg") -Destination $trimDir -Force
Write-Host "A) Copied sources -> $trimDir"

# Step B: trim (stroke width, metadata cleanup, enforce fill none)
& "$PSScriptRoot\trim_svgs.ps1" `
  -Src $trimDir `
  -Bak $trimBackup `
  -StrokeWidth $StrokeWidth `
  -ForceFillNone $ForceFillNone
Write-Host "B) Trimmed in place: $trimDir"

# Step C: copy svg_trim -> svg_outline and outlineify
Copy-Item -Path (Join-Path $trimDir "*.svg") -Destination $outlineDir -Force
& "$PSScriptRoot\outlineify_svgs.ps1" `
  -Src $outlineDir `
  -StrokeWidth $StrokeWidth
Write-Host "C) Outlineified in place: $outlineDir"

# Step D: copy svg_outline -> svg_final and fix clipPaths
Copy-Item -Path (Join-Path $outlineDir "*.svg") -Destination $finalDir -Force
& "$PSScriptRoot\fix_clipPath.ps1" `
  -Src $finalDir
Write-Host "D) clipPath fixed in place: $finalDir"

Write-Host "OK: svg_final ready in $finalDir (stroke-width=$StrokeWidth)"