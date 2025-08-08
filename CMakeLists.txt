cmake_minimum_required(VERSION 3.20)
include(CMakePrintHelpers)
project(sinen)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(SINEN_SOURCE_DIR ${CMAKE_SOURCE_DIR})

include(${SINEN_SOURCE_DIR}/cmake/extlibs_settings.cmake)
include(${SINEN_SOURCE_DIR}/cmake/include_dir.cmake)

add_subdirectory(${SINEN_SOURCE_DIR}/libs/SDL EXCLUDE_FROM_ALL)
add_subdirectory(${SINEN_SOURCE_DIR}/libs/SDL_image EXCLUDE_FROM_ALL)
add_subdirectory(${SINEN_SOURCE_DIR}/libs/SDL_ttf EXCLUDE_FROM_ALL)
add_subdirectory(${SINEN_SOURCE_DIR}/libs/assimp EXCLUDE_FROM_ALL)
add_subdirectory(${SINEN_SOURCE_DIR}/libs/SDL_mixer EXCLUDE_FROM_ALL)
add_subdirectory(${SINEN_SOURCE_DIR}/libs/paranoixa EXCLUDE_FROM_ALL)
add_subdirectory(${SINEN_SOURCE_DIR}/libs/zep EXCLUDE_FROM_ALL)
add_subdirectory(${SINEN_SOURCE_DIR}/libs/sol2 EXCLUDE_FROM_ALL)
add_subdirectory(${SINEN_SOURCE_DIR}/libs/luajit-cmake EXCLUDE_FROM_ALL)

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
else ()
    set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra -g -O0 -DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG -march=native")
endif ()
file(
        GLOB_RECURSE
        SINEN_SOURCE_FILES
        ${SINEN_SOURCE_DIR}/src/*.cpp
        ${SINEN_SOURCE_DIR}/libs/mojoAL/mojoal.c
)
file(
        GLOB_RECURSE
        SINEN_HEADER_FILES
        ${SINEN_SOURCE_DIR}/include/*.hpp
        ${SINEN_SOURCE_DIR}/include/*.inl
        ${SINEN_SOURCE_DIR}/*.hpp
)

add_executable(sinen docs/icon/icon.rc
        ${SINEN_SOURCE_FILES} ${SINEN_HEADER_FILES})

target_include_directories(
        sinen PUBLIC ${SINEN_INCLUDE_DIRS} ${SDL3_INCLUDE_PATHS})

target_link_libraries(
        sinen
        PUBLIC
        paranoixa
        paranoixa_imgui_backend
        SDL3::SDL3
        SDL3_image::SDL3_image
        SDL3_mixer::SDL3_mixer
        SDL3_ttf::SDL3_ttf
        assimp
        Zep::Zep
        luajit::lib
        luajit::header
)
target_compile_definitions(sinen PUBLIC
        AL_LIBTYPE_STATIC
        ZEP_USE_SDL
)

if (WIN32)
    if (MSVC)
        set(SUBSYSTEM_LINKER_OPTIONS "/SUBSYSTEM:WINDOWS")
    else ()
        set(SUBSYSTEM_LINKER_OPTIONS "-mwindows")
    endif ()
endif ()

target_link_options(sinen PRIVATE ${SUBSYSTEM_LINKER_OPTIONS})

set_property(TARGET sinen PROPERTY CXX_STANDARD 23)
