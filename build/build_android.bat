cmake --build %~dp0android/arm64-v8a_26_Release
copy %~dp0android\arm64-v8a_26_Release\lib\libsinen.so %~dp0android\app\src\main\jniLibs\arm64-v8a\libsinen.so