param(
  [string]$Src = "assets/icons/work/svg_final"
)

$ErrorActionPreference = "Stop"

Get-ChildItem -Path $Src -Filter *.svg | ForEach-Object {
  [xml]$xml = Get-Content $_.FullName -Raw

  $ns = New-Object System.Xml.XmlNamespaceManager($xml.NameTable)
  $ns.AddNamespace("svg", "http://www.w3.org/2000/svg")

  $clipPaths = $xml.SelectNodes("//svg:clipPath//svg:path", $ns)
  foreach ($p in $clipPaths) {
    $p.RemoveAttribute("stroke")
    $p.RemoveAttribute("stroke-width")
    $p.RemoveAttribute("stroke-linecap")
    $p.RemoveAttribute("stroke-linejoin")
    $p.RemoveAttribute("stroke-miterlimit")
    $p.RemoveAttribute("vector-effect")
  }

  $xml.Save($_.FullName)
  Write-Host "Fixed clipPath: $($_.Name)"
}