call "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64
cmake --build %~dp0msvc2026-release --parallel %NUMBER_OF_PROCESSORS%