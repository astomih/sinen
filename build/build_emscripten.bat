@echo off
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0xmake_build.ps1" -Profile emscripten
exit /b %errorlevel%
