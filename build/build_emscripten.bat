@echo off
setlocal
set "XMAKE_CONFIGDIR=%~dp0emscripten"
xmake build -P "%~dp0.." --all %*
exit /b %errorlevel%
