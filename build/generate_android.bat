@echo off
setlocal
set "XMAKE_CONFIGDIR=%~dp0android/arm64-v8a_28_Release"
if not defined ANDROID_NDK_ROOT (
  echo ANDROID_NDK_ROOT is required.
  exit /b 1
)
if not defined SINEN_SLANG_GENERATORS_PATH set "SINEN_SLANG_GENERATORS_PATH=%~dp0msvc2026-debug\vendor\upstream\slang\generators\Debug\bin"
xmake f -P "%~dp0.." -c -o "build/android/arm64-v8a_28_Release" -p android -a arm64-v8a -m release --ndk="%ANDROID_NDK_ROOT%" --ndk_sdkver=28 --runtimes=c++_static --sinen_cpp_tests=n --sinen_slang_generators="%SINEN_SLANG_GENERATORS_PATH%"
exit /b %errorlevel%
