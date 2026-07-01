@echo off
setlocal

set CURRENT_DIR=%~dp0
set BUILD_ABI=arm64-v8a
set BUILD_API_LEVEL=28
set BUILD_CONFIGURATION=Release
set BUILD_DIR=%CURRENT_DIR%android\%BUILD_ABI%_%BUILD_API_LEVEL%_%BUILD_CONFIGURATION%
set JNI_LIB_DIR=%CURRENT_DIR%android\app\src\main\jniLibs\%BUILD_ABI%

cmake --build "%BUILD_DIR%" --target sinen
if errorlevel 1 exit /b %ERRORLEVEL%

if not exist "%JNI_LIB_DIR%" mkdir "%JNI_LIB_DIR%"
if errorlevel 1 exit /b %ERRORLEVEL%

call :copy_lib "%BUILD_DIR%\lib\libsinen.so" "%JNI_LIB_DIR%\libsinen.so"
if errorlevel 1 exit /b %ERRORLEVEL%
call :copy_lib "%BUILD_DIR%\libs\SDL\lib\libSDL3.so" "%JNI_LIB_DIR%\libSDL3.so"
if errorlevel 1 exit /b %ERRORLEVEL%
call :copy_lib "%BUILD_DIR%\libs\assimp\code\lib\libassimp.so" "%JNI_LIB_DIR%\libassimp.so"
if errorlevel 1 exit /b %ERRORLEVEL%
call :copy_lib "%BUILD_DIR%\libs\cppdap\lib\libcppdap.so" "%JNI_LIB_DIR%\libcppdap.so"
if errorlevel 1 exit /b %ERRORLEVEL%
call :copy_lib "%BUILD_DIR%\libs\JoltPhysics\Build\lib\libJolt.so" "%JNI_LIB_DIR%\libJolt.so"
if errorlevel 1 exit /b %ERRORLEVEL%
call :copy_lib "%BUILD_DIR%\libs\slang\external\miniz\lib\libminiz.so" "%JNI_LIB_DIR%\libminiz.so"
if errorlevel 1 exit /b %ERRORLEVEL%
call :copy_lib "%BUILD_DIR%\libs\slang\source\slang\lib\libslang-compiler.so" "%JNI_LIB_DIR%\libslang-compiler.so"
if errorlevel 1 exit /b %ERRORLEVEL%
call :copy_lib "%BUILD_DIR%\libs\ktx-software\lib\lib\libktx_read.so" "%JNI_LIB_DIR%\libktx_read.so"
if errorlevel 1 exit /b %ERRORLEVEL%
call :copy_lib "%BUILD_DIR%\libs\msdfgen\lib\libmsdfgen-core.so" "%JNI_LIB_DIR%\libmsdfgen-core.so"
if errorlevel 1 exit /b %ERRORLEVEL%
call :copy_lib "%BUILD_DIR%\libs\tracy\lib\libTracyClient.so" "%JNI_LIB_DIR%\libTracyClient.so"
if errorlevel 1 exit /b %ERRORLEVEL%

exit /b 0

:copy_lib
if not exist "%~1" (
    echo Required library not found: %~1
    exit /b 1
)
copy /Y "%~1" "%~2"
if errorlevel 1 exit /b %ERRORLEVEL%
exit /b 0
