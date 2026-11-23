@echo off
if not "%~1"=="" set "SINEN_EMSCRIPTEN_PRELOAD_DIR=%~1"
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0xmake_configure.ps1" -Profile emscripten
exit /b %errorlevel%
