$src = "assets/icons/src_svg"
$strokeWidth = "1.8"

Get-ChildItem -Path $src -Filter *.svg | ForEach-Object {
    [xml]$xml = Get-Content $_.FullName -Raw

    $ns = New-Object System.Xml.XmlNamespaceManager($xml.NameTable)
    $ns.AddNamespace("svg", "http://www.w3.org/2000/svg")

    # Find all path elements
    $paths = $xml.SelectNodes("//svg:path", $ns)

    foreach ($p in $paths) {
        $hasStroke = $p.HasAttribute("stroke")
        $hasFill   = $p.HasAttribute("fill")

        # If the path has no stroke, turn it into an outline path
        if (-not $hasStroke) {
            $p.SetAttribute("stroke", "#000")
            $p.SetAttribute("stroke-width", $strokeWidth)
            $p.SetAttribute("stroke-linecap", "round")
            $p.SetAttribute("stroke-linejoin", "round")
            $p.SetAttribute("vector-effect", "non-scaling-stroke")

            # Ensure outline (no solid fill)
            $p.SetAttribute("fill", "none")
        }
        else {
            # If it already has stroke, still enforce outline fill
            $p.SetAttribute("fill", "none")
        }
    }

    $xml.Save($_.FullName)
    Write-Host "Outlineified: $($_.Name)"
}