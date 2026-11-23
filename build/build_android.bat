@echo off
setlocal
set "XMAKE_CONFIGDIR=%~dp0android/arm64-v8a_28_Release"
xmake build -P "%~dp0.." --all %*
exit /b %errorlevel%
