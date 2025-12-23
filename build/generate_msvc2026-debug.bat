call "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64
cmake %~dp0.. -B %~dp0msvc2026-debug -G "Ninja" -DCMAKE_BUILD_TYPE=Debug