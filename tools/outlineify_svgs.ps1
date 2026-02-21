param(
  [string]$Src = "assets/icons/work/svg_outline",
  [string]$StrokeWidth = "2.2"
)

$ErrorActionPreference = "Stop"

Get-ChildItem -Path $Src -Filter *.svg | ForEach-Object {
  [xml]$xml = Get-Content $_.FullName -Raw
  $ns = New-Object System.Xml.XmlNamespaceManager($xml.NameTable)
  $ns.AddNamespace("svg", "http://www.w3.org/2000/svg")

  $paths = $xml.SelectNodes("//svg:path", $ns)
  foreach ($p in $paths) {
    if (-not $p.HasAttribute("stroke")) {
      $p.SetAttribute("stroke", "#000")
      $p.SetAttribute("stroke-width", $StrokeWidth)
      $p.SetAttribute("stroke-linecap", "round")
      $p.SetAttribute("stroke-linejoin", "round")
      $p.SetAttribute("vector-effect", "non-scaling-stroke")
      $p.SetAttribute("fill", "none")
    } else {
      $p.SetAttribute("fill", "none")
      $p.SetAttribute("stroke-width", $StrokeWidth)
    }
  }

  $xml.Save($_.FullName)
  Write-Host "Outlineified: $($_.Name)"
}