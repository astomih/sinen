@REM build_and_run.bat

@echo off

set CURRENT_DIR=%~dp0

@REM Check SDK
if not defined ANDROID_SDK_ROOT (
    echo "The environment variable ANDROID_SDK_ROOT is not defined."
    exit /b -1
)

if not exist %ANDROID_SDK_ROOT% (
    echo "ANDROID_SDK_ROOT=%ANDROID_SDK_ROOT% is not exist."
    exit /b -1
)

@REM Check NDK
if not defined ANDROID_NDK_ROOT (
    echo "The environment variable ANDROID_NDK_ROOT is not defined."
    exit /b -1
)

if not exist %ANDROID_NDK_ROOT% (
    echo "ANDROID_SDK_ROOT=%ANDROID_NDK_ROOT% is not exist."
    exit /b -1
)

@REM Check CMake
set CMAKE_VERSION=4.1.1
set ANDROID_CMAKE_PATH=%ANDROID_SDK_ROOT%\cmake\%CMAKE_VERSION%\bin
if not exist %ANDROID_CMAKE_PATH%\cmake.exe (
    echo "CMake (%CMAKE_VERSION%) is not available."
    exit /b -1
)

@REM Set build target
set BUILD_ABI=arm64-v8a
set BUILD_API_LEVEL=26
set BUILD_CONFIGURATION=Release

set BUILD_DIR=%CURRENT_DIR%\android\%BUILD_ABI%_%BUILD_API_LEVEL%_%BUILD_CONFIGURATION%

@REM Generate
%ANDROID_CMAKE_PATH%\cmake.exe ^
-GNinja ^
-DCMAKE_SYSTEM_NAME=Android ^
-DCMAKE_EXPORT_COMPILE_COMMANDS=ON ^
-DCMAKE_SYSTEM_VERSION=%BUILD_API_LEVEL% ^
-DANDROID_PLATFORM=android-%BUILD_API_LEVEL% ^
-DANDROID_ABI=%BUILD_ABI% ^
-DCMAKE_ANDROID_ARCH_ABI=%BUILD_ABI% ^
-DANDROID_NDK=%ANDROID_NDK_ROOT% ^
-DCMAKE_ANDROID_NDK=%ANDROID_NDK_ROOT% ^
-DCMAKE_TOOLCHAIN_FILE=%ANDROID_NDK_ROOT%\build\cmake\android.toolchain.cmake ^
-DCMAKE_MAKE_PROGRAM=%ANDROID_CMAKE_PATH%\ninja.exe ^
-DCMAKE_CXX_FLAGS=-std=c++23 -Wno ^
-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=lib ^
-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=bin ^
-DCMAKE_BUILD_TYPE=%BUILD_CONFIGURATION% ^
-DANDROID_STL=c++_static ^
-S.. ^
-B%BUILD_DIR%
