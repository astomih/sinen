cmake %~dp0.. -B %~dp0vs2022 -G "Visual Studio 17 2022"
mklink /D vs2022\works\data ..\..\..\works\data
mklink /D vs2022\works\script ..\..\..\works\script
mklink /D vs2022\editor\docs ..\..\..\docs
mklink /D vs2022\editor\data ..\..\..\editor\data
mklink /D vs2022\editor\script ..\..\..\editor\script
