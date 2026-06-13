@echo on
copy "%~dp0msvc2026-release\sinen.exe" "%~dp0..\src\gpu\shader\default\sinen.exe"
cd /d "%~dp0../src/gpu/shader/default"
sinen