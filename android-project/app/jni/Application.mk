
# Uncomment this if you're using STL in your project
# You can find more information here:
# https://developer.android.com/ndk/guides/cpp-support
APP_STL := c++_static

#APP_ABI := armeabi-v7a arm64-v8a x86 x86_64
APP_ABI := arm64-v8a
APP_CPPFLAGS := -std=c++20 -w -frtti
# Min runtime API level
APP_PLATFORM=android-30
