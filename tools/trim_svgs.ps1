$src = "assets/icons/src_svg"
$bak = "assets/icons/src_svg_backup"
$strokeWidth = "1.8"   # <-- tweak: 1.6 / 1.8 / 2.0
$forceFillNone = $true # outline-friendly

New-Item -ItemType Directory -Force -Path $bak | Out-Null

Get-ChildItem -Path $src -Filter *.svg | ForEach-Object {
    $inFile  = $_.FullName
    $outFile = $inFile
    $bakFile = Join-Path $bak $_.Name

    Copy-Item $inFile $bakFile -Force

    [xml]$xml = Get-Content $inFile -Raw

    # SVG namespace handling
    $ns = New-Object System.Xml.XmlNamespaceManager($xml.NameTable)
    $ns.AddNamespace("svg", "http://www.w3.org/2000/svg")

    # Select elements that have a stroke attribute (and not stroke="none")
    $nodes = $xml.SelectNodes("//*[@stroke]", $ns)

    foreach ($n in $nodes) {
        $stroke = $n.GetAttribute("stroke")
        if ($stroke -and $stroke.ToLower() -ne "none") {
            $n.SetAttribute("stroke-width", $strokeWidth)
            $n.SetAttribute("stroke-linecap", "round")
            $n.SetAttribute("stroke-linejoin", "round")
            # Optional but useful if you ever scale the SVG
            $n.SetAttribute("vector-effect", "non-scaling-stroke")

            if ($forceFillNone) {
                # Avoid accidental fills that can look muddy on e-ink
                $fill = $n.GetAttribute("fill")
                if (-not $fill -or $fill.ToLower() -ne "none") {
                    $n.SetAttribute("fill", "none")
                }
            }
        }
    }

    # Remove common metadata nodes if present (keeps it compatible with many converters)
    $removeNames = @("metadata","title","desc")
    foreach ($name in $removeNames) {
        $toRemove = $xml.SelectNodes("//svg:$name", $ns)
        foreach ($r in $toRemove) {
            $null = $r.ParentNode.RemoveChild($r)
        }
    }

    $xml.Save($outFile)
    Write-Host "Trimmed: $($_.Name)"
}

Write-Host "Done. Backups in: $bak"