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
option(SINEN_ENABLE_TRACY "Enable Tracy CPU frame profiling" ON)
option(SINEN_EMSCRIPTEN_ASYNCIFY
       "Enable Emscripten Asyncify support for blocking browser waits" ON)
option(SINEN_EMSCRIPTEN_ASSERTIONS
       "Enable Emscripten runtime assertions in generated JavaScript" OFF)
option(SINEN_EMSCRIPTEN_DEBUG_RUNTIME
       "Enable Emscripten runtime checks and source maps for debugging" OFF)
option(SINEN_EMSCRIPTEN_EXCEPTIONS
       "Enable C++ exception catching in Emscripten builds" ON)
set(SINEN_EMSCRIPTEN_PRELOAD_DIR
    ""
    CACHE PATH "Directory to preload into the Emscripten virtual filesystem")
set(SINEN_EMSCRIPTEN_PRELOAD_MOUNT
    "/"
    CACHE STRING "Virtual filesystem mount point for SINEN_EMSCRIPTEN_PRELOAD_DIR")

if(EMSCRIPTEN AND SINEN_EMSCRIPTEN_EXCEPTIONS)
  add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:-fexceptions>")
  add_link_options(-fexceptions -sNO_DISABLE_EXCEPTION_CATCHING=1)
endif()

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
  target_link_options(webgpu INTERFACE --use-port=emdawnwebgpu)
  if(SINEN_EMSCRIPTEN_ASYNCIFY)
    target_link_options(webgpu INTERFACE -sASYNCIFY=1)
  endif()
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

add_subdirectory(${SINEN_SOURCE_DIR}/libs/luau)
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

set(MESHOPT_BUILD_DEMO OFF CACHE BOOL "Build meshoptimizer demo" FORCE)
set(MESHOPT_BUILD_GLTFPACK OFF CACHE BOOL "Build gltfpack" FORCE)
set(MESHOPT_INSTALL OFF CACHE BOOL "Install meshoptimizer" FORCE)
add_subdirectory(${SINEN_SOURCE_DIR}/libs/meshoptimizer EXCLUDE_FROM_ALL)

set(KTX_FEATURE_DOC OFF CACHE BOOL "Create KTX documentation" FORCE)
set(KTX_FEATURE_JNI OFF CACHE BOOL "Create Java bindings for libktx" FORCE)
set(KTX_FEATURE_PY OFF CACHE BOOL "Create Python source distribution" FORCE)
set(KTX_FEATURE_TESTS OFF CACHE BOOL "Create KTX unit tests" FORCE)
set(KTX_FEATURE_TOOLS OFF CACHE BOOL "Enable KTX CLI tools" FORCE)
set(KTX_FEATURE_TOOLS_CTS OFF CACHE BOOL "Enable KTX CLI tools CTS tests" FORCE)
set(KTX_FEATURE_LOADTEST_APPS OFF CACHE STRING "Build KTX load test apps" FORCE)
set(LIBKTX_FEATURE_KTX1 ON CACHE BOOL "Enable KTX 1 support" FORCE)
set(LIBKTX_FEATURE_KTX2 ON CACHE BOOL "Enable KTX 2 support" FORCE)
set(LIBKTX_FEATURE_VK_UPLOAD OFF CACHE BOOL "Enable Vulkan upload helpers" FORCE)
set(LIBKTX_FEATURE_GL_UPLOAD OFF CACHE BOOL "Enable OpenGL upload helpers" FORCE)
add_subdirectory(${SINEN_SOURCE_DIR}/libs/ktx-software EXCLUDE_FROM_ALL)

if(SINEN_ENABLE_TRACY)
  set(TRACY_ENABLE ON CACHE BOOL "Enable Tracy profiling" FORCE)
  set(TRACY_ON_DEMAND ON CACHE BOOL "Enable Tracy on-demand profiling" FORCE)
  set(TRACY_ONLY_LOCALHOST ON CACHE BOOL "Restrict Tracy to localhost" FORCE)
  add_subdirectory(${SINEN_SOURCE_DIR}/libs/tracy EXCLUDE_FROM_ALL)
endif()

if(SINEN_USE_SLANG)
  add_subdirectory(${SINEN_SOURCE_DIR}/libs/slang EXCLUDE_FROM_ALL)
  if(MSVC AND TARGET libcmark-gfm)
    target_compile_options(
      libcmark-gfm
      PUBLIC $<$<COMPILE_LANG_AND_ID:C,MSVC>:/utf-8>
             $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/utf-8>)
  endif()
endif()

add_subdirectory(${SINEN_SOURCE_DIR}/src)
