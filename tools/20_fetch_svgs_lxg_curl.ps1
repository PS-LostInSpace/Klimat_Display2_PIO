param(
  [string]$Dst = "assets/icons/sources/lxg_weather_icons_3.0.1"
)

$ErrorActionPreference = "Stop"
New-Item -ItemType Directory -Force -Path $Dst | Out-Null

$BASE = "https://unpkg.com/@lxg/weather-icons@3.0.1/production/monochrome/svg-static"

$map = @{
  "clear-day.svg"         = "sun.svg"
  "clear-night.svg"       = "clear-night.svg"
  "partly-cloudy-day.svg" = "partly.svg"
  "cloudy.svg"            = "cloud.svg"
  "rain.svg"              = "rain.svg"
  "sleet.svg"             = "sleet.svg"
  "snow.svg"              = "snow.svg"
  "fog.svg"               = "fog.svg"
  "wind.svg"              = "wind.svg"
  "not-available.svg"     = "unknown.svg"
}

foreach ($k in $map.Keys) {
  $url = "$BASE/$k"
  $out = Join-Path $Dst $map[$k]
  & curl.exe -L --fail --silent --show-error $url -o $out
  $fi = Get-Item $out
  Write-Host ("OK  {0,-16} {1,6} bytes  {2}" -f $fi.Name, $fi.Length, $fi.LastWriteTime)
}

Write-Host "Done. Output=$Dst"