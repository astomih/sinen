@echo off
setlocal
set "XMAKE_CONFIGDIR=%~dp0emscripten"
if not "%~1"=="" set "SINEN_EMSCRIPTEN_PRELOAD_DIR=%~f1"
xmake f -P "%~dp0.." -c -o build/emscripten -p wasm -a wasm32 -m release --toolchain=emcc --sinen_shader_compiler=n --sinen_preload="%SINEN_EMSCRIPTEN_PRELOAD_DIR%"
exit /b %errorlevel%
