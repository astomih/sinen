cmake %~dp0.. -B %~dp0vs2022 -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Debug
cmake --build %~dp0vs2022 --config Debug