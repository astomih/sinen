$ErrorActionPreference = 'Stop'
$out = Join-Path $PSScriptRoot 'android/arm64-v8a_28_Release'
$dest = Join-Path $PSScriptRoot 'android/app/src/main/jniLibs/arm64-v8a'
New-Item -ItemType Directory -Force -Path $dest | Out-Null
$names = @('libsinen.so', 'libshader_compiler.so', 'libSDL3.so',
           'libassimp.so', 'libcppdap.so', 'libbox2d.so', 'libbox3d.so',
           'libminiz.so', 'libslang-compiler.so', 'libktx_read.so',
           'libmsdfgen-core.so', 'libTracyClient.so')
foreach ($name in $names) {
    $file = Get-ChildItem -LiteralPath $out -Recurse -File -Filter $name |
        Where-Object { $_.FullName -notmatch '[\\/]app[\\/]' } |
        Select-Object -First 1
    if (-not $file) { throw "Required Android library not found: $name" }
    Copy-Item -LiteralPath $file.FullName -Destination (Join-Path $dest $name) -Force
}
