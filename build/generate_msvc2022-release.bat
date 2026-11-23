@echo off
setlocal
set "XMAKE_CONFIGDIR=%~dp0msvc2022-release"
xmake f -P "%~dp0.." -c -o "build/msvc2022-release" -p windows -a x64 -m release --vs=2022 %*
exit /b %errorlevel%
