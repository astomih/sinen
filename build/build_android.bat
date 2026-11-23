@echo off
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0xmake_build.ps1" -Profile android/arm64-v8a_28_Release
if errorlevel 1 exit /b %errorlevel%
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0xmake_stage_android.ps1"
exit /b %errorlevel%
