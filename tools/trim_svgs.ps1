param(
  [string]$Src = "assets/icons/work/svg_trim",
  [string]$Bak = "assets/icons/work/svg_trim_backup",
  [string]$StrokeWidth = "2.2",
  [bool]$ForceFillNone = $true
)

$ErrorActionPreference = "Stop"

New-Item -ItemType Directory -Force -Path $Bak | Out-Null

Get-ChildItem -Path $Src -Filter *.svg | ForEach-Object {
  Copy-Item $_.FullName (Join-Path $Bak $_.Name) -Force

  [xml]$xml = Get-Content $_.FullName -Raw
  $ns = New-Object System.Xml.XmlNamespaceManager($xml.NameTable)
  $ns.AddNamespace("svg", "http://www.w3.org/2000/svg")

  $nodes = $xml.SelectNodes("//*[@stroke]", $ns)
  foreach ($n in $nodes) {
    $stroke = $n.GetAttribute("stroke")
    if ($stroke -and $stroke.ToLower() -ne "none") {
      $n.SetAttribute("stroke-width", $StrokeWidth)
      $n.SetAttribute("stroke-linecap", "round")
      $n.SetAttribute("stroke-linejoin", "round")
      $n.SetAttribute("vector-effect", "non-scaling-stroke")

      if ($ForceFillNone) {
        $fill = $n.GetAttribute("fill")
        if (-not $fill -or $fill.ToLower() -ne "none") {
          $n.SetAttribute("fill", "none")
        }
      }
    }
  }

  foreach ($name in @("metadata","title","desc")) {
    $toRemove = $xml.SelectNodes("//svg:$name", $ns)
    foreach ($r in $toRemove) { $null = $r.ParentNode.RemoveChild($r) }
  }

  $xml.Save($_.FullName)
  Write-Host "Trimmed: $($_.Name)"
}

Write-Host "Done. Backups in: $Bak"