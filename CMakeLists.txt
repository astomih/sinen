cmake_minimum_required(VERSION 3.20)
include(CMakePrintHelpers)
project(sinen)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(SINEN_SOURCE_DIR ${CMAKE_SOURCE_DIR})

include(${SINEN_SOURCE_DIR}/cmake/extlibs_settings.cmake)
include(${SINEN_SOURCE_DIR}/cmake/include_dir.cmake)

add_subdirectory(${SINEN_SOURCE_DIR}/libs/SDL EXCLUDE_FROM_ALL)
add_subdirectory(${SINEN_SOURCE_DIR}/libs/assimp EXCLUDE_FROM_ALL)
add_subdirectory(${SINEN_SOURCE_DIR}/libs/paranoixa EXCLUDE_FROM_ALL)
add_subdirectory(${SINEN_SOURCE_DIR}/libs/zep EXCLUDE_FROM_ALL)
add_subdirectory(${SINEN_SOURCE_DIR}/libs/sol2 EXCLUDE_FROM_ALL)
add_subdirectory(${SINEN_SOURCE_DIR}/libs/JoltPhysics/Build EXCLUDE_FROM_ALL)
option(SINEN_USE_SLANG "Use Slang for shader compilation" ON)

if (SINEN_USE_SLANG)
    add_subdirectory(${SINEN_SOURCE_DIR}/libs/slang EXCLUDE_FROM_ALL)
endif ()

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
else ()
    set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra -g -O0 -DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG -Wno-format-security")
endif ()
file(
        GLOB_RECURSE
        SINEN_SOURCE_FILES
        ${SINEN_SOURCE_DIR}/src/*.cpp
        ${SINEN_SOURCE_DIR}/libs/lua/onelua.c
        ${SINEN_SOURCE_DIR}/libs/tinyexr/deps/miniz/miniz.c
)
file(
        GLOB_RECURSE
        SINEN_HEADER_FILES
        ${SINEN_SOURCE_DIR}/include/*.hpp
        ${SINEN_SOURCE_DIR}/include/*.inl
        ${SINEN_SOURCE_DIR}/*.hpp
)

if (BUILD_SHARED_LIBS)
    add_library(sinen SHARED docs/icon/icon.rc
            ${SINEN_SOURCE_FILES} ${SINEN_HEADER_FILES})
else ()
    add_executable(sinen docs/icon/icon.rc
            ${SINEN_SOURCE_FILES} ${SINEN_HEADER_FILES})
endif ()
target_include_directories(
        sinen PUBLIC ${SINEN_INCLUDE_DIRS} ${SDL3_INCLUDE_PATHS})

set(SINEN_LINK_LIBRARIES
        paranoixa
        paranoixa_imgui_backend
        SDL3::SDL3
        assimp
        Zep::Zep
        Jolt
)

if (SINEN_USE_SLANG)
    list(APPEND SINEN_LINK_LIBRARIES slang)
endif ()

if (ANDROID)
    find_library(ANDROID_LIB android)
    list(APPEND SINEN_LINK_LIBRARIES ${ANDROID_LIB})
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
