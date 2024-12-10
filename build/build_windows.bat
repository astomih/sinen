REM make resource's symlink to build directory
mklink /D msvc\works\data ..\..\..\works\data
mklink /D msvc\works\script ..\..\..\works\script
mklink /D msvc\editor\docs ..\..\..\docs
mklink /D msvc\editor\data ..\..\..\editor\data
mklink /D msvc\editor\script ..\..\..\editor\script
cmake -B msvc .. -G "Visual Studio 17 2022" -DBUILD_SHARED_LIBS=OFF
