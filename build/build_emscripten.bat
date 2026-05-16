@echo off
setlocal

set "BUILD_DIR=%~dp0emscripten"

where emcmake >nul 2>nul
if errorlevel 1 (
  if defined EMSDK (
    if exist "%EMSDK%\emsdk_env.bat" (
      call "%EMSDK%\emsdk_env.bat"
    )
  )
)

if not exist "%BUILD_DIR%\CMakeCache.txt" (
  echo Emscripten build directory is not configured. Run generate_emscripten.bat first.
  exit /b 1
)

sudo cmake --build "%BUILD_DIR%" --parallel %NUMBER_OF_PROCESSORS%
