param(
    [Parameter(Mandatory=$true)][string]$Profile,
    [string]$Target = '',
    [switch]$VerboseBuild,
    [switch]$RunTests,
    [string]$TestPattern = ''
)
$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$out = Join-Path $PSScriptRoot $Profile
$env:SINEN_OUTPUT_DIR = $out
$env:VSLANG = '1033'
if ($Profile -eq 'emscripten') {
    $preloadFile = Join-Path $out 'xmake-assets.txt'
    if (Test-Path $preloadFile) {
        $env:SINEN_EMSCRIPTEN_PRELOAD_DIR = Get-Content -LiteralPath $preloadFile -Raw
    }
}
$vendor = Join-Path $out 'vendor'
if (-not (Test-Path (Join-Path $vendor 'CMakeCache.txt'))) {
    throw "Run the matching generate_*.bat first: $Profile"
}
$cmakeArgs = @('--build', $vendor, '--target', 'sinen_vendor', '--parallel')
if ($env:NUMBER_OF_PROCESSORS) { $cmakeArgs += $env:NUMBER_OF_PROCESSORS }
& cmake @cmakeArgs
if ($LASTEXITCODE -ne 0) { throw "Vendor build failed with exit code $LASTEXITCODE" }
if ($Profile -like 'msvc*' -or $Profile -eq 'windows-release') {
    $slangCompiler = Get-ChildItem -LiteralPath $vendor -Recurse -File -Filter 'slang-compiler.dll' |
        Select-Object -First 1
    if ($slangCompiler) {
        Copy-Item -LiteralPath $slangCompiler.FullName -Destination (Join-Path $out 'slang-compiler.dll') -Force
    }
}
Push-Location $root
try {
    $configFile = Join-Path $out 'xmake-config.txt'
    if (-not (Test-Path $configFile)) {
        throw "Run the matching generate_*.bat first: $Profile"
    }
    & xmake f -c "--import=$configFile"
    if ($LASTEXITCODE -ne 0) { throw "xmake configure import failed with exit code $LASTEXITCODE" }
    $xmakeArgs = @('build')
    if ($VerboseBuild) { $xmakeArgs += '-v' }
    if ($Target) { $xmakeArgs += $Target } else { $xmakeArgs += '--all' }
    & xmake @xmakeArgs
    if ($LASTEXITCODE -ne 0) { throw "xmake build failed with exit code $LASTEXITCODE" }
    if ($RunTests) {
        $testArgs = @('test', '--output-on-failure')
        if ($TestPattern) {
            if ($TestPattern -notmatch '[/\\*?]') { $TestPattern += '/*' }
            $testArgs += $TestPattern
        }
        & xmake @testArgs
        if ($LASTEXITCODE -ne 0) { throw "xmake tests failed with exit code $LASTEXITCODE" }
    }
} finally { Pop-Location }
