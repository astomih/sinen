cmake -B msvc .. -G "Visual Studio 17 2022" -DBUILD_SHARED_LIBS=OFF

REM make resource's symlink to build directory
mklink /D msvc\works\data ..\..\..\works\data
mklink /D msvc\works\script ..\..\..\works\script
mklink msvc\works\settings.json ..\..\..\works\settings.json