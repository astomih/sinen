@echo on
copy "%~dp0msvc2026-release\sinen.exe" "%~dp0..\src\gpu\default\sinen.exe"
copy "%~dp0msvc2026-release\webgpu_dawn.dll" "%~dp0..\src\gpu\default\webgpu_dawn.dll"
cd /d "%~dp0../src/gpu/default"
sinen