powershell -executionpolicy RemoteSigned -File "%~dp0../libs/SDL_image/external/Get-GitModules.ps1"
cmake %~dp0.. -B %~dp0vs2022 -G "Visual Studio 17 2022"
mklink /D %~dp0vs2022\works\data ..\..\..\works\data
mklink /D %~dp0vs2022\works\script ..\..\..\works\script
mklink /D %~dp0vs2022\editor\docs ..\..\..\docs
mklink /D %~dp0vs2022\editor\data ..\..\..\editor\data
mklink /D %~dp0vs2022\editor\script ..\..\..\editor\script
