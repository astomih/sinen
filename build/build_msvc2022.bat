call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64
cmake --build %~dp0msvc2022 --config Debug --parallel %NUMBER_OF_PROCESSORS%