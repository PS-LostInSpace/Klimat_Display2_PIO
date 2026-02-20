$src = "assets/icons/src_svg"

Get-ChildItem -Path $src -Filter *.svg | ForEach-Object {
    [xml]$xml = Get-Content $_.FullName -Raw

    $ns = New-Object System.Xml.XmlNamespaceManager($xml.NameTable)
    $ns.AddNamespace("svg", "http://www.w3.org/2000/svg")

    # All paths inside any clipPath
    $clipPaths = $xml.SelectNodes("//svg:clipPath//svg:path", $ns)

    foreach ($p in $clipPaths) {
        $p.RemoveAttribute("stroke")
        $p.RemoveAttribute("stroke-width")
        $p.RemoveAttribute("stroke-linecap")
        $p.RemoveAttribute("stroke-linejoin")
        $p.RemoveAttribute("stroke-miterlimit")
        $p.RemoveAttribute("vector-effect")
        # Keep fill as none or leave it untouched; clip paths usually work with fill.
        if ($p.HasAttribute("fill") -and $p.GetAttribute("fill").ToLower() -eq "none") {
            # ok
        }
    }

    $xml.Save($_.FullName)
    Write-Host "Fixed clipPath: $($_.Name)"
}