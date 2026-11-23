@echo off
setlocal
set "XMAKE_CONFIGDIR=%~dp0msvc2026-debug"
xmake f -P "%~dp0.." -c -o "build/msvc2026-debug" -p windows -a x64 -m debug --vs=2026 --sinen_webgpu_dawn=y %*
exit /b %errorlevel%
