@echo off
setlocal

set "BUILD_DIR=%~dp0emscripten"
set "BUILD_TYPE=Release"
set "ASSET_DIR=%~1"
set "ASSET_OPTIONS="
set CURRENT_DIR=%~dp0

if not "%ASSET_DIR%"=="" (
  set "ASSET_OPTIONS=-DSINEN_EMSCRIPTEN_PRELOAD_DIR=%ASSET_DIR% -DSINEN_EMSCRIPTEN_PRELOAD_MOUNT=/"
)

where emcmake >nul 2>nul
if errorlevel 1 (
  if defined EMSDK (
    if exist "%EMSDK%\emsdk_env.bat" (
      call "%EMSDK%\emsdk_env.bat"
    )
  )
)

where emcmake >nul 2>nul
if errorlevel 1 (
  echo emcmake was not found. Activate emsdk first, or set EMSDK to your emsdk directory.
  exit /b 1
)

if not exist "%BUILD_DIR%" (
  mkdir "%BUILD_DIR%"
)

@REM Slang on Emscripten is currently disabled due to some issues with the Slang compiler. It may be re-enabled in the future when those issues are resolved. 

emcmake cmake ^
-S "%~dp0.." ^
-B "%BUILD_DIR%" ^
-G Ninja ^
-DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
-DCMAKE_EXECUTABLE_SUFFIX=.html ^
-DBUILD_SHARED_LIBS=OFF ^
-DSINEN_MODULE_SHADER_COMPILER=OFF ^ 
-DSINEN_EMSCRIPTEN_ASYNCIFY=ON ^
-DSINEN_EMSCRIPTEN_ASSERTIONS=ON ^
-DSINEN_EMSCRIPTEN_DEBUG_RUNTIME=OFF ^
-DSINEN_EMSCRIPTEN_EXCEPTIONS=ON ^
%ASSET_OPTIONS% ^
-DASSIMP_BUILD_ZLIB=ON ^
