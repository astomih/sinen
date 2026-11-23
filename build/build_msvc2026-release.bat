@echo off
setlocal
set "XMAKE_CONFIGDIR=%~dp0msvc2026-release"
if /i "%~1"=="-RunTests" (
  if /i "%~2"=="-TestPattern" (
    xmake test -P "%~dp0.." --output-on-failure "%~3/*"
  ) else (
    xmake test -P "%~dp0.." --output-on-failure
  )
  if errorlevel 1 exit /b 1
  exit /b 0
)
if "%~1"=="" (
  xmake build -P "%~dp0.." --all
) else (
  xmake build -P "%~dp0.." %*
)
exit /b %errorlevel%
