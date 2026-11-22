param(
    [string]$SourceDirectory,
    [string]$Archive
)
$ErrorActionPreference = 'Stop'
$repo = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '../../..')).Path
$cache = Join-Path $repo 'build/msvc2026-debug/bistro-source'
if (-not $SourceDirectory) {
    New-Item -ItemType Directory -Path $cache -Force | Out-Null
    if (-not $Archive) { $Archive = Join-Path $cache 'Bistro_v5_2.zip' }
    if (-not (Test-Path -LiteralPath $Archive)) {
        Write-Host 'Downloading the official ORCA archive (approximately 853 MiB)...'
        $partial = "$Archive.partial"
        Invoke-WebRequest -Uri 'https://developer.nvidia.com/bistro' -OutFile $partial
        $expected = '0D50E3C724C6C5DA19F8EB99AD3F53E36FEC37FFA2DF9621F9CCF0603F3934E1'
        if ((Get-FileHash -LiteralPath $partial -Algorithm SHA256).Hash -ne $expected) {
            throw 'Archive checksum changed. Check the ORCA source before using the downloaded file.'
        }
        Move-Item -LiteralPath $partial -Destination $Archive
    }
    $SourceDirectory = Join-Path $cache 'Bistro_v5_2'
    if (-not (Test-Path -LiteralPath (Join-Path $SourceDirectory 'BistroExterior.fbx'))) {
        Add-Type -AssemblyName System.IO.Compression.FileSystem
        [System.IO.Compression.ZipFile]::ExtractToDirectory((Resolve-Path -LiteralPath $Archive).Path, $cache)
    }
}
$source = (Resolve-Path -LiteralPath $SourceDirectory).Path
if (-not (Test-Path -LiteralPath (Join-Path $source 'BistroExterior.fbx'))) {
    throw 'SourceDirectory must contain the original BistroExterior.fbx and Textures directory.'
}
$destination = Join-Path $PSScriptRoot 'assets/Bistro_v5_2'
New-Item -ItemType Directory -Path $destination -Force | Out-Null
foreach ($name in @('BistroExterior.fbx', 'Textures', 'san_giuseppe_bridge_4k.hdr', 'LICENSE.txt', 'README.txt')) {
    Copy-Item -LiteralPath (Join-Path $source $name) -Destination $destination -Recurse -Force
}
Write-Host 'Ready: original FBX/DDS assets copied without conversion. Launch Examples > Graphics > Bistro (Advanced).'
