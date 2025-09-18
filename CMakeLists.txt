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
add_subdirectory(${SINEN_SOURCE_DIR}/libs/JoltPhysics/Build EXCLUDE_FROM_ALL)
option(SINEN_USE_SLANG "Use Slang for shader compilation" OFF)

if (SINEN_USE_SLANG)
    add_subdirectory(${SINEN_SOURCE_DIR}/libs/slang EXCLUDE_FROM_ALL)
endif ()

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
        ${SINEN_SOURCE_DIR}/libs/lua/onelua.c
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

set(SINEN_LINK_LIBRARIES
        paranoixa
        paranoixa_imgui_backend
        SDL3::SDL3
        SDL3_image::SDL3_image
        SDL3_mixer::SDL3_mixer
        SDL3_ttf::SDL3_ttf
        assimp
        Zep::Zep
        Jolt
)

if (SINEN_USE_SLANG)
    list(APPEND SINEN_LINK_LIBRARIES slang)
endif ()

target_link_libraries(
        sinen
        PUBLIC
        ${SINEN_LINK_LIBRARIES}
)
target_compile_definitions(sinen PUBLIC
        AL_LIBTYPE_STATIC
        ZEP_USE_SDL
        MAKE_LIB
        $<$<BOOL:${SINEN_USE_SLANG}>:SINEN_USE_SLANG>
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
