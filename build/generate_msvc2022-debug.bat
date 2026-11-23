@echo off
setlocal
set "XMAKE_CONFIGDIR=%~dp0msvc2022-debug"
xmake f -P "%~dp0.." -c -o "build/msvc2022-debug" -p windows -a x64 -m debug --vs=2022 --sinen_webgpu_dawn=y %*
exit /b %errorlevel%
