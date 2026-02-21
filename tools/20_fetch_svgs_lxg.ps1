# tools/20_fetch_svgs_lxg.ps1 (v2)
param(
  [string]$Dst = "assets/icons/sources/lxg_weather_icons_3.0.1",
  [int]$TimeoutSec = 30
)

$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"

# Some corporate environments need TLS 1.2 explicitly (mostly PS 5.1)
try {
  [Net.ServicePointManager]::SecurityProtocol = `
    [Net.ServicePointManager]::SecurityProtocol -bor `
    [Net.SecurityProtocolType]::Tls12
} catch {}

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

# Proxy detection (helps on locked-down machines)
$proxy = $null
try {
  $proxy = [System.Net.WebRequest]::DefaultWebProxy
  if ($proxy -and $proxy.GetProxy([Uri]"https://unpkg.com").AbsoluteUri -ne "https://unpkg.com/") {
    Write-Host "Proxy detected: $($proxy.GetProxy([Uri]'https://unpkg.com').AbsoluteUri)"
  } else {
    $proxy = $null
  }
} catch { $proxy = $null }

$ok = 0
$fail = 0

foreach ($k in $map.Keys) {
  $url = "$BASE/$k"
  $out = Join-Path $Dst $map[$k]

  try {
    if ($proxy) {
      Invoke-WebRequest `
        -Uri $url `
        -OutFile $out `
        -UseBasicParsing `
        -TimeoutSec $TimeoutSec `
        -Proxy $proxy.GetProxy([Uri]$url).AbsoluteUri `
        -ProxyUseDefaultCredentials
    } else {
      Invoke-WebRequest `
        -Uri $url `
        -OutFile $out `
        -UseBasicParsing `
        -TimeoutSec $TimeoutSec
    }

    $fi = Get-Item $out
    Write-Host ("OK  {0,-16} {1,6} bytes  {2}" -f $fi.Name, $fi.Length, $fi.LastWriteTime)
    $ok++
  }
  catch {
    Write-Host ("FAIL {0,-16}  {1}" -f $map[$k], $_.Exception.Message) -ForegroundColor Red
    $fail++
  }
}

Write-Host "Done. OK=$ok FAIL=$fail  Output=$Dst"
if ($fail -gt 0) { exit 1 }