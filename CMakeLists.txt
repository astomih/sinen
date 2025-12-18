cmake_minimum_required(VERSION 3.20)
include(CMakePrintHelpers)
project(sinen)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(SINEN_SOURCE_DIR ${CMAKE_SOURCE_DIR})
if (NOT EMSCRIPTEN)
    find_package(Vulkan REQUIRED)
endif ()

include(${SINEN_SOURCE_DIR}/cmake/extlibs_settings.cmake)
include(${SINEN_SOURCE_DIR}/cmake/include_dir.cmake)

add_subdirectory(${SINEN_SOURCE_DIR}/libs/SDL EXCLUDE_FROM_ALL)
add_subdirectory(${SINEN_SOURCE_DIR}/libs/assimp EXCLUDE_FROM_ALL)
add_subdirectory(${SINEN_SOURCE_DIR}/libs/sol2 EXCLUDE_FROM_ALL)
add_subdirectory(${SINEN_SOURCE_DIR}/libs/JoltPhysics/Build EXCLUDE_FROM_ALL)
if (WIN32)
    set(CMAKE_FOLDER "webgpu")
    add_subdirectory(${SINEN_SOURCE_DIR}/libs/WebGPU-distribution EXCLUDE_FROM_ALL)
    unset(CMAKE_FOLDER)
endif ()
if (NOT EMSCRIPTEN)
    add_subdirectory(${SINEN_SOURCE_DIR}/libs/LuaJIT EXCLUDE_FROM_ALL)
    if (MSVC)
        target_compile_definitions(libluajit PRIVATE _CRT_SECURE_NO_WARNINGS)
        target_compile_definitions(minilua PRIVATE _CRT_SECURE_NO_WARNINGS)
        target_compile_definitions(buildvm PRIVATE _CRT_SECURE_NO_WARNINGS)
    endif ()
endif ()
option(SINEN_USE_SLANG "Use Slang for shader compilation" ON)

if (SINEN_USE_SLANG)
    add_subdirectory(${SINEN_SOURCE_DIR}/libs/slang EXCLUDE_FROM_ALL)
endif ()

set(CMAKE_CXX_FLAGS "-DIMGUI_IMPL_VULKAN_NO_PROTOTYPES -DIMGUI_IMPL_WEBGPU_BACKEND_WGPU")
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
        ${SINEN_SOURCE_DIR}/libs/tinyexr/deps/miniz/miniz.c
        ${SINEN_SOURCE_DIR}/libs/tlsf/tlsf.c

        ${SINEN_SOURCE_DIR}/libs/imgui/imgui.cpp
        ${SINEN_SOURCE_DIR}/libs/imgui/imgui_demo.cpp
        ${SINEN_SOURCE_DIR}/libs/imgui/imgui_draw.cpp
        ${SINEN_SOURCE_DIR}/libs/imgui/imgui_tables.cpp
        ${SINEN_SOURCE_DIR}/libs/imgui/imgui_widgets.cpp
        ${SINEN_SOURCE_DIR}/libs/imgui/backends/imgui_impl_sdl3.cpp
)
if (NOT ANDROID)
    list(APPEND
            SINEN_SOURCE_FILES
            ${SINEN_SOURCE_DIR}/libs/imgui/backends/imgui_impl_wgpu.cpp)
endif ()
if (WIN32)
    list(APPEND
            SINEN_SOURCE_FILES
            ${SINEN_SOURCE_DIR}/libs/imgui/backends/imgui_impl_dx12.cpp)
endif ()
if (EMSCRIPTEN)
    list(APPEND
            SINEN_SOURCE_FILES
            ${SINEN_SOURCE_DIR}/libs/lua/onelua.c)
else ()
    list(APPEND
            SINEN_SOURCE_FILES
            ${SINEN_SOURCE_DIR}/libs/imgui/backends/imgui_impl_vulkan.cpp)
endif ()
file(
        GLOB_RECURSE
        SINEN_HEADER_FILES
        ${SINEN_SOURCE_DIR}/src/*.hpp
        ${SINEN_SOURCE_DIR}/src/*.inl
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
    if (MSVC)
  # MSVC STL
  set(MSVC_STL_INCLUDE
      "$ENV{VCToolsInstallDir}/include")

  # Windows SDK
  set(WINSDK_INCLUDE_BASE
      "$ENV{WindowsSdkDir}/Include/$ENV{WindowsSDKVersion}")

  set(WINSDK_INCLUDES
      "${WINSDK_INCLUDE_BASE}/ucrt"
      "${WINSDK_INCLUDE_BASE}/shared"
      "${WINSDK_INCLUDE_BASE}/um")

  target_include_directories(sinen SYSTEM PRIVATE
      "${MSVC_STL_INCLUDE}"
      ${WINSDK_INCLUDES})
endif()

set(SINEN_LINK_LIBRARIES
        SDL3::SDL3
        assimp
        Jolt
)

if (SINEN_USE_SLANG)
    list(APPEND SINEN_LINK_LIBRARIES slang)
endif ()

if (NOT EMSCRIPTEN)
    list(APPEND SINEN_LINK_LIBRARIES libluajit)
endif ()

if (WIN32)
    list(APPEND SINEN_LINK_LIBRARIES d3d12)
    list(APPEND SINEN_LINK_LIBRARIES dxgi)
    list(APPEND SINEN_LINK_LIBRARIES Ws2_32)
endif ()

if (ANDROID)
    find_library(ANDROID_LIB android)
    list(APPEND SINEN_LINK_LIBRARIES ${ANDROID_LIB})
else ()
    target_copy_webgpu_binaries(sinen)
    list(APPEND SINEN_LINK_LIBRARIES webgpu)
endif ()

target_link_libraries(
        sinen
        PUBLIC
        ${SINEN_LINK_LIBRARIES}
)
target_compile_definitions(sinen PUBLIC
        AL_LIBTYPE_STATIC
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
