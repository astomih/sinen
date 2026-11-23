@echo off
if not defined ANDROID_SDK_ROOT (
  echo ANDROID_SDK_ROOT is required.
  exit /b 1
)
if not defined ANDROID_NDK_ROOT (
  echo ANDROID_NDK_ROOT is required.
  exit /b 1
)
if not defined SINEN_SLANG_GENERATORS_PATH (
  set "SINEN_SLANG_GENERATORS_PATH=%~dp0msvc2026-debug\vendor\generators\Debug\bin"
)
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0xmake_configure.ps1" -Profile android/arm64-v8a_28_Release
exit /b %errorlevel%
