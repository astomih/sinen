param(
    [Parameter(Mandatory=$true)][string]$Profile,
    [string]$Assets = ""
)
$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$out = Join-Path $PSScriptRoot $Profile
$vendor = Join-Path $out 'vendor'
$env:SINEN_OUTPUT_DIR = $out
New-Item -ItemType Directory -Force -Path $vendor | Out-Null

$cmakeArgs = @('-S', (Join-Path $root 'cmake/vendor'), '-B', $vendor,
               '-G', 'Ninja', '-DCMAKE_POLICY_VERSION_MINIMUM=3.5')
$xmakeArgs = @('f', '-c', '-o', $out, "--export=$(Join-Path $out 'xmake-config.txt')")
if ($Profile -eq 'emscripten') {
    $preloadFile = Join-Path $out 'xmake-assets.txt'
    if ($env:SINEN_EMSCRIPTEN_PRELOAD_DIR) {
        $assetPath = (Resolve-Path $env:SINEN_EMSCRIPTEN_PRELOAD_DIR).Path
        Set-Content -LiteralPath $preloadFile -Value $assetPath -NoNewline
        $env:SINEN_EMSCRIPTEN_PRELOAD_DIR = $assetPath
    } elseif (Test-Path $preloadFile) {
        $env:SINEN_EMSCRIPTEN_PRELOAD_DIR = Get-Content -LiteralPath $preloadFile -Raw
    }
    $cmakeArgs += @('-DCMAKE_BUILD_TYPE=Release', '-DSINEN_VENDOR_SLANG=OFF')
    & emcmake cmake @cmakeArgs
    $xmakeArgs += @('-p', 'wasm', '-a', 'wasm32', '-m', 'release', '--toolchain=emcc',
                    '--sinen_shader_compiler=n', '--sinen_webgpu_dawn=n')
} elseif ($Profile -like 'android/*' -or $Profile -like 'android\*') {
    if (-not $env:ANDROID_NDK_ROOT) { throw 'ANDROID_NDK_ROOT is required' }
    $cmakeArgs += @('-DCMAKE_BUILD_TYPE=Release', '-DBUILD_SHARED_LIBS=ON',
        '-DSINEN_VENDOR_TESTS=OFF',
        '-DCMAKE_SYSTEM_NAME=Android', '-DANDROID_ABI=arm64-v8a',
        '-DANDROID_PLATFORM=android-28', '-DANDROID_STL=c++_static',
        "-DCMAKE_TOOLCHAIN_FILE=$env:ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake")
    if ($env:SINEN_SLANG_GENERATORS_PATH) {
        $cmakeArgs += "-DSLANG_GENERATORS_PATH=$env:SINEN_SLANG_GENERATORS_PATH"
    }
    & cmake @cmakeArgs
    $xmakeArgs += @('-p', 'android', '-a', 'arm64-v8a', '-m', 'release',
                    "--ndk=$env:ANDROID_NDK_ROOT", '--ndk_sdkver=28',
                    '--runtimes=c++_static',
                    '--sinen_webgpu_dawn=n', '--sinen_cpp_tests=n')
} else {
    $mode = if ($Profile.EndsWith('debug')) { 'debug' } else { 'release' }
    $dawn = if ($mode -eq 'debug') { 'y' } else { 'n' }
    $cmakeArgs += @("-DCMAKE_BUILD_TYPE=$($mode.Substring(0,1).ToUpper()+$mode.Substring(1))",
                    "-DSINEN_ENABLE_NATIVE_WEBGPU_DAWN=$dawn")
    & cmake @cmakeArgs
    $xmakeArgs += @('-p', 'windows', '-a', 'x64', '-m', $mode,
                    "--sinen_webgpu_dawn=$dawn")
    if ($Profile -match '^msvc(2022|2026)') {
        $xmakeArgs += "--vs=$($Matches[1])"
    }
}
if ($LASTEXITCODE -ne 0) { throw "Vendor configure failed with exit code $LASTEXITCODE" }
Push-Location $root
try {
    & xmake @xmakeArgs
    if ($LASTEXITCODE -ne 0) { throw "xmake configure failed with exit code $LASTEXITCODE" }
} finally { Pop-Location }
