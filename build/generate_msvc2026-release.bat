@echo off
setlocal
set "XMAKE_CONFIGDIR=%~dp0msvc2026-release"
xmake f -P "%~dp0.." -c -o "build/msvc2026-release" -p windows -a x64 -m release --vs=2026 %*
exit /b %errorlevel%
