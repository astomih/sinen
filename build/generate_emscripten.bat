@echo off
setlocal

set "BUILD_DIR=%~dp0emscripten"
set "BUILD_TYPE=Release"
set "ASSET_DIR=%~1"

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

emcmake cmake ^
  -S "%~dp0.." ^
  -B "%BUILD_DIR%" ^
  -G Ninja ^
  -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
  -DCMAKE_EXECUTABLE_SUFFIX=.html ^
  -DBUILD_SHARED_LIBS=OFF ^
  -DSINEN_USE_SLANG=OFF ^
  -DSINEN_EMSCRIPTEN_ASYNCIFY=ON ^
  -DSINEN_EMSCRIPTEN_ASSERTIONS=OFF ^
  -DSINEN_EMSCRIPTEN_DEBUG_RUNTIME=OFF ^
  -DSINEN_EMSCRIPTEN_PRELOAD_DIR="%ASSET_DIR%" ^
  -DSINEN_EMSCRIPTEN_PRELOAD_MOUNT=/ ^
  -DASSIMP_BUILD_ZLIB=ON
