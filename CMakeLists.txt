cmake_minimum_required(VERSION 3.20)
include(CMakePrintHelpers)
project(sinen)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(SINEN_SOURCE_DIR ${CMAKE_SOURCE_DIR})
if(NOT EMSCRIPTEN)
  find_package(Vulkan REQUIRED)
endif()

option(SINEN_USE_SLANG "Use Slang for shader compilation" ON)
option(SINEN_ENABLE_NATIVE_WEBGPU_DAWN
       "Enable native WebGPU backend through Dawn" ON)

include(${SINEN_SOURCE_DIR}/cmake/include.cmake)
include(${SINEN_SOURCE_DIR}/cmake/libs.cmake)

add_subdirectory(${SINEN_SOURCE_DIR}/libs/SDL EXCLUDE_FROM_ALL)
add_subdirectory(${SINEN_SOURCE_DIR}/libs/assimp EXCLUDE_FROM_ALL)
add_subdirectory(${SINEN_SOURCE_DIR}/libs/JoltPhysics/Build EXCLUDE_FROM_ALL)
set(SINEN_ENABLE_WEBGPU OFF)
if(EMSCRIPTEN AND NOT TARGET webgpu)
  add_library(webgpu INTERFACE)
  target_compile_definitions(webgpu INTERFACE WEBGPU_BACKEND_EMDAWNWEBGPU)
  target_compile_options(webgpu INTERFACE --use-port=emdawnwebgpu)
  target_link_options(webgpu INTERFACE --use-port=emdawnwebgpu -sASYNCIFY=1)
  function(target_copy_webgpu_binaries Target)
  endfunction()
  set(SINEN_ENABLE_WEBGPU ON)
elseif(SINEN_ENABLE_NATIVE_WEBGPU_DAWN AND WIN32 AND NOT TARGET webgpu)
  set(WEBGPU_BACKEND DAWN CACHE STRING "Native WebGPU backend" FORCE)
  set(WEBGPU_LINK_TYPE SHARED CACHE STRING "Native WebGPU link type" FORCE)
  set(SINEN_PREVIOUS_CMAKE_FOLDER ${CMAKE_FOLDER})
  set(CMAKE_FOLDER "webgpu")
  add_subdirectory(${SINEN_SOURCE_DIR}/libs/WebGPU-distribution
                   EXCLUDE_FROM_ALL)
  set(CMAKE_FOLDER ${SINEN_PREVIOUS_CMAKE_FOLDER})
  unset(SINEN_PREVIOUS_CMAKE_FOLDER)
  set(SINEN_ENABLE_WEBGPU ON)
endif()

add_subdirectory(${SINEN_SOURCE_DIR}/libs/Luau)
foreach(target Luau.VM Luau.Compiler)
  get_target_property(defs ${target} INTERFACE_COMPILE_DEFINITIONS)
  if(defs)
    list(FILTER defs EXCLUDE REGEX [[LUA.*_API]])
    set_target_properties(${target} PROPERTIES INTERFACE_COMPILE_DEFINITIONS
                                               "${defs}")
  endif()
endforeach()
set_target_properties(Luau.Analysis Luau.CLI.lib Luau.CodeGen Luau.Config
                      Luau.Require PROPERTIES EXCLUDE_FROM_ALL 1)
add_subdirectory(${SINEN_SOURCE_DIR}/libs/cppdap)
add_subdirectory(${SINEN_SOURCE_DIR}/libs/luau-debugger/debugger)
add_subdirectory(${SINEN_SOURCE_DIR}/libs/msdfgen EXCLUDE_FROM_ALL)

if(SINEN_USE_SLANG)
  add_subdirectory(${SINEN_SOURCE_DIR}/libs/slang EXCLUDE_FROM_ALL)
endif()

file(
  GLOB_RECURSE
  SINEN_SOURCE_FILES
  ${SINEN_SOURCE_DIR}/src/*.cpp
  ${SINEN_SOURCE_DIR}/libs/tinyexr/deps/miniz/miniz.c
  ${SINEN_SOURCE_DIR}/libs/tlsf/tlsf.c
)

if(NOT SINEN_ENABLE_WEBGPU)
  list(FILTER SINEN_SOURCE_FILES EXCLUDE REGEX
       ".*[\\\\/]src[\\\\/]gpu[\\\\/]webgpu[\\\\/].*\\.cpp$")
endif()
file(GLOB_RECURSE SINEN_HEADER_FILES ${SINEN_SOURCE_DIR}/src/*.hpp
     ${SINEN_SOURCE_DIR}/src/*.inl ${SINEN_SOURCE_DIR}/*.hpp)

set(SINEN_PLATFORM_SOURCES)
if(WIN32)
  list(APPEND SINEN_PLATFORM_SOURCES docs/icon/icon.rc)
endif()

if(BUILD_SHARED_LIBS)
  add_library(sinen SHARED ${SINEN_PLATFORM_SOURCES} ${SINEN_SOURCE_FILES}
                           ${SINEN_HEADER_FILES})
else()
  add_executable(sinen ${SINEN_PLATFORM_SOURCES} ${SINEN_SOURCE_FILES}
                       ${SINEN_HEADER_FILES})
endif()
if(EMSCRIPTEN AND NOT BUILD_SHARED_LIBS)
  set_target_properties(sinen PROPERTIES SUFFIX ".html")
endif()
target_include_directories(sinen PUBLIC ${SINEN_INCLUDE_DIRS}
                                        ${SDL3_INCLUDE_PATHS})
if(TARGET dawn::webgpu_dawn)
  get_target_property(SINEN_DAWN_INCLUDE_DIRS dawn::webgpu_dawn
                      INTERFACE_INCLUDE_DIRECTORIES)
  if(SINEN_DAWN_INCLUDE_DIRS)
    target_include_directories(sinen BEFORE PRIVATE ${SINEN_DAWN_INCLUDE_DIRS})
  endif()
endif()
target_compile_definitions(
  sinen
  PUBLIC $<$<BOOL:${SINEN_USE_SLANG}>:SINEN_USE_SLANG>)
if(EMSCRIPTEN)
  target_compile_definitions(sinen PUBLIC EMSCRIPTEN)
  target_link_options(
    sinen
    PRIVATE
      -sALLOW_MEMORY_GROWTH=1
      -sINITIAL_MEMORY=268435456
      -sMAXIMUM_MEMORY=2147483648
      -sASSERTIONS=1)
endif()
if(SINEN_ENABLE_WEBGPU)
  target_compile_definitions(sinen PUBLIC SINEN_ENABLE_WEBGPU)
endif()
if(MSVC)
  target_compile_options(sinen PRIVATE /bigobj)
else()
  target_compile_options(
    sinen
    PRIVATE $<$<CONFIG:Debug>:-Wall;-Wextra;-g;-O0>
            $<$<CONFIG:Release>:-O2;-Wno-format-security>)
  target_compile_definitions(sinen PRIVATE $<$<CONFIG:Debug>:DEBUG>
                                           $<$<CONFIG:Release>:NDEBUG>)
endif()
if(MSVC)
  # MSVC STL
  set(MSVC_STL_INCLUDE "$ENV{VCToolsInstallDir}/include")

  # Windows SDK
  set(WINSDK_INCLUDE_BASE "$ENV{WindowsSdkDir}/Include/$ENV{WindowsSDKVersion}")

  set(WINSDK_INCLUDES
      "${WINSDK_INCLUDE_BASE}/ucrt" "${WINSDK_INCLUDE_BASE}/shared"
      "${WINSDK_INCLUDE_BASE}/um" "${WINSDK_INCLUDE_BASE}/winrt")

  target_include_directories(sinen SYSTEM PRIVATE "${MSVC_STL_INCLUDE}"
                                                  ${WINSDK_INCLUDES})
endif()

set(SINEN_LINK_LIBRARIES SDL3::SDL3 assimp Jolt)

if(SINEN_ENABLE_WEBGPU)
  list(APPEND SINEN_LINK_LIBRARIES webgpu)
endif()

if(SINEN_USE_SLANG)
  list(APPEND SINEN_LINK_LIBRARIES slang)
endif()

list(
  APPEND
  SINEN_LINK_LIBRARIES
  Luau.VM
  Luau.Compiler
  Luau.Ast
  Luau.Require
  cppdap
  msdfgen-core
  Luau.Debugger)

if(WIN32)
  list(APPEND SINEN_LINK_LIBRARIES d3d12)
  list(APPEND SINEN_LINK_LIBRARIES dxgi)
  list(APPEND SINEN_LINK_LIBRARIES dxguid)
  list(APPEND SINEN_LINK_LIBRARIES Ws2_32)
endif()

if(ANDROID)
  find_library(ANDROID_LIB android)
  list(APPEND SINEN_LINK_LIBRARIES ${ANDROID_LIB})
endif()

if(SINEN_ENABLE_WEBGPU)
  target_copy_webgpu_binaries(sinen)
endif()

target_link_libraries(sinen PUBLIC ${SINEN_LINK_LIBRARIES})

if(WIN32)
  if(MSVC)
    set(SUBSYSTEM_LINKER_OPTIONS "/SUBSYSTEM:WINDOWS")
  else()
    set(SUBSYSTEM_LINKER_OPTIONS "-mwindows")
  endif()
endif()

target_link_options(sinen PRIVATE ${SUBSYSTEM_LINKER_OPTIONS})

set_property(TARGET sinen PROPERTY CXX_STANDARD 23)
