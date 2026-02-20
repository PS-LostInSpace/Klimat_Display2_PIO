param(
  [string]$DeviceUrl = "http://192.168.1.224",
  [string]$OtaUser = "admin",
  [string]$OtaPass = "AppKD2PIO,2026!",
  [string]$PioEnv = "reterminal_e1001",
  [switch]$SkipBuild,
  [int]$VerifyRetries = 30,
  [int]$VerifyDelaySeconds = 2
)

$ErrorActionPreference = "Stop"

function Invoke-CurlWithStatus {
  param(
    [Parameter(Mandatory = $true)][string[]]$CurlArgs
  )

  $response = & curl.exe @CurlArgs
  if ($LASTEXITCODE -ne 0) {
    throw "curl failed with exit code $LASTEXITCODE"
  }

  $responseText = [string]::Join("`n", @($response))

  $status = 0
  $body = $responseText

  $match = [regex]::Match($responseText, "HTTP_STATUS:(\d{3})")
  if ($match.Success) {
    $status = [int]$match.Groups[1].Value
    $body = ($responseText -replace "HTTP_STATUS:\d{3}", "").Trim()
  }

  [PSCustomObject]@{
    Status = $status
    Body = $body
    Raw = $responseText
  }
}

Write-Host "== OTA upload script =="
Write-Host "Device: $DeviceUrl"
Write-Host "PIO env: $PioEnv"

if (-not $SkipBuild) {
  Write-Host "[1/5] Building firmware..."
  & pio run -e $PioEnv
  if ($LASTEXITCODE -ne 0) {
    throw "Build failed"
  }
} else {
  Write-Host "[1/5] Build skipped (--SkipBuild)"
}

Write-Host "[2/5] Locating firmware and calculating MD5..."
$firmwarePath = Join-Path (Get-Location) ".pio/build/$PioEnv/firmware.bin"
if (-not (Test-Path $firmwarePath)) {
  throw "Firmware not found: $firmwarePath"
}

$md5 = (Get-FileHash -Algorithm MD5 $firmwarePath).Hash.ToLower()
Write-Host "Firmware: $firmwarePath"
Write-Host "MD5: $md5"

Write-Host "[3/5] Starting OTA session..."
$start = Invoke-CurlWithStatus -CurlArgs @(
  "-sS",
  "-w", "`nHTTP_STATUS:%{http_code}`n",
  "-u", "$OtaUser`:$OtaPass",
  "$DeviceUrl/ota/start?mode=fr&hash=$md5"
)

if ($start.Status -ne 200) {
  throw "OTA start failed (HTTP $($start.Status)): $($start.Body)"
}

Write-Host "Start response: $($start.Body)"

Write-Host "[4/5] Uploading firmware..."
$upload = Invoke-CurlWithStatus -CurlArgs @(
  "-sS",
  "-w", "`nHTTP_STATUS:%{http_code}`n",
  "-u", "$OtaUser`:$OtaPass",
  "-F", "file=@$firmwarePath",
  "$DeviceUrl/ota/upload"
)

if ($upload.Status -ne 200) {
  throw "OTA upload failed (HTTP $($upload.Status)): $($upload.Body)"
}

Write-Host "Upload accepted by device. Waiting for reboot..."

Write-Host "[5/5] Verifying device after reboot..."
$ok = $false
for ($i = 1; $i -le $VerifyRetries; $i++) {
  Start-Sleep -Seconds $VerifyDelaySeconds

  try {
    $health = Invoke-CurlWithStatus -CurlArgs @(
      "-sS",
      "-w", "`nHTTP_STATUS:%{http_code}`n",
      "$DeviceUrl/health"
    )

    if ($health.Status -eq 200 -and $health.Body -match '"ok"\s*:\s*true') {
      Write-Host "Health OK on attempt $i/$VerifyRetries"
      $ok = $true
      break
    }
  } catch {
    # Device may still be rebooting
  }
}

if (-not $ok) {
  throw "Upload was sent, but device did not report healthy within timeout."
}

Write-Host "OTA completed successfully."