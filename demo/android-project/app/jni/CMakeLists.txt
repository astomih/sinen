cmake_minimum_required(VERSION 3.6)

project(demo)

# armeabi-v7a requires cpufeatures library
include(AndroidNdkModules)
android_ndk_import_module_cpufeatures()


# SDL sources are in a subfolder named "SDL"
add_subdirectory(SDL2)
add_subdirectory(SDL2_image)
add_subdirectory(SDL2_mixer)
add_subdirectory(SDL2_ttf)
add_subdirectory(SDL2_net)
add_subdirectory(sinen)
add_subdirectory(demo)
