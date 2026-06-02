cmake_minimum_required(VERSION 3.20)
include(CMakePrintHelpers)
project(sinen)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(SINEN_SOURCE_DIR ${CMAKE_SOURCE_DIR})

option(SINEN_ENABLE_NATIVE_WEBGPU_DAWN
       "Enable native WebGPU backend through Dawn" ON)
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

option(SINEN_BUILD_APP "Build the Sinen application executable" ON)
option(SINEN_MODULE_CORE "Build the Core module" ON)
option(SINEN_MODULE_MATH "Build the Math module" ON)
option(SINEN_MODULE_PLATFORM "Build the Platform module" ON)
option(SINEN_MODULE_AUDIO "Build the Audio module" ON)
option(SINEN_MODULE_GPU "Build the GPU module" ON)
option(SINEN_MODULE_SHADER_COMPILER "Build the ShaderCompiler module" ON)
option(SINEN_MODULE_GRAPHICS "Build the Graphics module" ON)
option(SINEN_MODULE_COMPUTE "Build the Compute module" ON)
option(SINEN_MODULE_PHYSICS "Build the Physics module" ON)
option(SINEN_MODULE_SCRIPT "Build the Script module" ON)
option(SINEN_MODULE_PROFILER "Build the profiler integration module" ON)

function(sinen_require_module module dependency)
  if(SINEN_MODULE_${module} AND NOT SINEN_MODULE_${dependency})
    message(FATAL_ERROR
            "SINEN_MODULE_${module} requires SINEN_MODULE_${dependency}=ON")
  endif()
endfunction()

function(sinen_require_app_module module)
  if(SINEN_BUILD_APP AND NOT SINEN_MODULE_${module})
    message(FATAL_ERROR
            "SINEN_BUILD_APP requires SINEN_MODULE_${module}=ON")
  endif()
endfunction()

sinen_require_module(MATH CORE)
sinen_require_module(PLATFORM CORE)
sinen_require_module(PLATFORM MATH)
sinen_require_module(AUDIO CORE)
sinen_require_module(AUDIO MATH)
sinen_require_module(AUDIO PLATFORM)
sinen_require_module(GPU CORE)
sinen_require_module(GPU MATH)
sinen_require_module(SHADER_COMPILER CORE)
sinen_require_module(SHADER_COMPILER GPU)
sinen_require_module(SHADER_COMPILER PLATFORM)
sinen_require_module(GRAPHICS CORE)
sinen_require_module(GRAPHICS MATH)
sinen_require_module(GRAPHICS GPU)
sinen_require_module(GRAPHICS PLATFORM)
sinen_require_module(COMPUTE CORE)
sinen_require_module(COMPUTE GPU)
sinen_require_module(COMPUTE GRAPHICS)
sinen_require_module(PHYSICS CORE)
sinen_require_module(PHYSICS MATH)
sinen_require_module(SCRIPT CORE)
sinen_require_module(SCRIPT MATH)
sinen_require_module(SCRIPT PLATFORM)
sinen_require_module(SCRIPT AUDIO)
sinen_require_module(SCRIPT GPU)
sinen_require_module(SCRIPT GRAPHICS)
sinen_require_module(SCRIPT COMPUTE)
sinen_require_module(SCRIPT PHYSICS)
if(SINEN_MODULE_PROFILER AND NOT SINEN_MODULE_CORE)
  message(FATAL_ERROR "SINEN_MODULE_PROFILER=ON requires SINEN_MODULE_CORE=ON")
endif()

sinen_require_app_module(CORE)
sinen_require_app_module(MATH)
sinen_require_app_module(PLATFORM)
sinen_require_app_module(AUDIO)
sinen_require_app_module(GPU)
sinen_require_app_module(GRAPHICS)
sinen_require_app_module(PHYSICS)
sinen_require_app_module(SCRIPT)

if(SINEN_MODULE_GPU AND NOT EMSCRIPTEN)
  find_package(Vulkan REQUIRED)
endif()

if(EMSCRIPTEN AND SINEN_EMSCRIPTEN_EXCEPTIONS)
  add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:-fexceptions>")
  add_link_options(-fexceptions -sNO_DISABLE_EXCEPTION_CATCHING=1)
endif()

include(${SINEN_SOURCE_DIR}/cmake/include.cmake)
include(${SINEN_SOURCE_DIR}/cmake/libs.cmake)

if(SINEN_MODULE_CORE OR SINEN_MODULE_PLATFORM OR SINEN_MODULE_AUDIO
   OR SINEN_MODULE_GPU OR SINEN_MODULE_GRAPHICS OR SINEN_MODULE_SCRIPT
   OR SINEN_BUILD_APP)
  add_subdirectory(${SINEN_SOURCE_DIR}/libs/SDL EXCLUDE_FROM_ALL)
endif()

if(SINEN_MODULE_GRAPHICS)
  add_subdirectory(${SINEN_SOURCE_DIR}/libs/assimp EXCLUDE_FROM_ALL)
endif()

if(SINEN_MODULE_PHYSICS)
  add_subdirectory(${SINEN_SOURCE_DIR}/libs/JoltPhysics/Build EXCLUDE_FROM_ALL)
endif()

set(SINEN_ENABLE_WEBGPU OFF)
if(SINEN_MODULE_GPU AND EMSCRIPTEN AND NOT TARGET webgpu)
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
elseif(SINEN_MODULE_GPU AND SINEN_ENABLE_NATIVE_WEBGPU_DAWN AND WIN32
       AND NOT TARGET webgpu)
  set(WEBGPU_BACKEND DAWN CACHE STRING "Native WebGPU backend" FORCE)
  set(WEBGPU_LINK_TYPE SHARED CACHE STRING "Native WebGPU link type" FORCE)
  set(WEBGPU_BUILD_FROM_SOURCE OFF CACHE BOOL
      "Build WebGPU backend from source" FORCE)
  set(SINEN_PREVIOUS_CMAKE_FOLDER ${CMAKE_FOLDER})
  set(CMAKE_FOLDER "webgpu")
  add_subdirectory(${SINEN_SOURCE_DIR}/libs/WebGPU-distribution
                   EXCLUDE_FROM_ALL)
  set(CMAKE_FOLDER ${SINEN_PREVIOUS_CMAKE_FOLDER})
  unset(SINEN_PREVIOUS_CMAKE_FOLDER)
  set(SINEN_ENABLE_WEBGPU ON)
endif()

if(SINEN_MODULE_SCRIPT)
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
endif()

if(SINEN_MODULE_GRAPHICS)
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
endif()

if(SINEN_MODULE_PROFILER)
  set(TRACY_ENABLE ON CACHE BOOL "Enable Tracy profiling" FORCE)
  set(TRACY_ON_DEMAND ON CACHE BOOL "Enable Tracy on-demand profiling" FORCE)
  set(TRACY_ONLY_LOCALHOST ON CACHE BOOL "Restrict Tracy to localhost" FORCE)
  add_subdirectory(${SINEN_SOURCE_DIR}/libs/tracy EXCLUDE_FROM_ALL)
endif()

if(SINEN_MODULE_SHADER_COMPILER)
  add_subdirectory(${SINEN_SOURCE_DIR}/libs/slang EXCLUDE_FROM_ALL)
  if(MSVC AND TARGET libcmark-gfm)
    target_compile_options(
      libcmark-gfm
      PUBLIC $<$<COMPILE_LANG_AND_ID:C,MSVC>:/utf-8>
             $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/utf-8>)
  endif()
endif()

add_library(sinen_common INTERFACE)
target_include_directories(sinen_common INTERFACE ${SINEN_INCLUDE_DIRS})

target_compile_definitions(
  sinen_common
  INTERFACE $<$<CONFIG:Debug>:SINEN_DEBUG_WORKING_DIRECTORY>)

foreach(SINEN_MODULE_NAME CORE MATH PLATFORM AUDIO GPU SHADER_COMPILER GRAPHICS
                          COMPUTE PHYSICS SCRIPT PROFILER)
  if(SINEN_MODULE_${SINEN_MODULE_NAME})
    target_compile_definitions(
      sinen_common INTERFACE SINEN_MODULE_${SINEN_MODULE_NAME})
  endif()
endforeach()

if(SINEN_ENABLE_WEBGPU)
  target_compile_definitions(sinen_common INTERFACE SINEN_ENABLE_WEBGPU)
endif()

if(EMSCRIPTEN)
  target_compile_definitions(sinen_common INTERFACE EMSCRIPTEN)
  if(SINEN_EMSCRIPTEN_EXCEPTIONS)
    target_compile_options(sinen_common INTERFACE -fexceptions)
  endif()
  if(SINEN_EMSCRIPTEN_DEBUG_RUNTIME)
    target_compile_options(sinen_common INTERFACE -gsource-map)
  endif()
endif()

if(MSVC)
  target_compile_options(sinen_common INTERFACE /bigobj)

  set(SINEN_MSVC_SYSTEM_INCLUDES)

  if(DEFINED ENV{VCToolsInstallDir} AND EXISTS "$ENV{VCToolsInstallDir}/include")
    list(APPEND SINEN_MSVC_SYSTEM_INCLUDES "$ENV{VCToolsInstallDir}/include")
  else()
    get_filename_component(SINEN_MSVC_BIN_ARCH_DIR "${CMAKE_CXX_COMPILER}"
                           DIRECTORY)
    get_filename_component(SINEN_MSVC_BIN_HOST_DIR "${SINEN_MSVC_BIN_ARCH_DIR}"
                           DIRECTORY)
    get_filename_component(SINEN_MSVC_BIN_DIR "${SINEN_MSVC_BIN_HOST_DIR}"
                           DIRECTORY)
    get_filename_component(SINEN_MSVC_TOOL_DIR "${SINEN_MSVC_BIN_DIR}" DIRECTORY)
    if(EXISTS "${SINEN_MSVC_TOOL_DIR}/include")
      list(APPEND SINEN_MSVC_SYSTEM_INCLUDES "${SINEN_MSVC_TOOL_DIR}/include")
    endif()
  endif()

  if(DEFINED ENV{WindowsSdkDir} AND DEFINED ENV{WindowsSDKVersion})
    set(SINEN_WINSDK_INCLUDE_BASE
        "$ENV{WindowsSdkDir}/Include/$ENV{WindowsSDKVersion}")
  elseif(CMAKE_MT)
    get_filename_component(SINEN_WINSDK_BIN_ARCH_DIR "${CMAKE_MT}" DIRECTORY)
    get_filename_component(SINEN_WINSDK_VERSION_DIR "${SINEN_WINSDK_BIN_ARCH_DIR}"
                           DIRECTORY)
    get_filename_component(SINEN_WINSDK_BIN_DIR "${SINEN_WINSDK_VERSION_DIR}"
                           DIRECTORY)
    get_filename_component(SINEN_WINSDK_ROOT_DIR "${SINEN_WINSDK_BIN_DIR}"
                           DIRECTORY)
    get_filename_component(SINEN_WINSDK_VERSION "${SINEN_WINSDK_VERSION_DIR}"
                           NAME)
    set(SINEN_WINSDK_INCLUDE_BASE
        "${SINEN_WINSDK_ROOT_DIR}/Include/${SINEN_WINSDK_VERSION}")
  endif()

  foreach(SINEN_WINSDK_INCLUDE ucrt shared um winrt)
    if(EXISTS "${SINEN_WINSDK_INCLUDE_BASE}/${SINEN_WINSDK_INCLUDE}")
      list(APPEND SINEN_MSVC_SYSTEM_INCLUDES
           "${SINEN_WINSDK_INCLUDE_BASE}/${SINEN_WINSDK_INCLUDE}")
    endif()
  endforeach()

  if(SINEN_MSVC_SYSTEM_INCLUDES)
    target_include_directories(sinen_common SYSTEM
                               INTERFACE ${SINEN_MSVC_SYSTEM_INCLUDES})
  endif()
else()
  target_compile_options(
    sinen_common
    INTERFACE $<$<CONFIG:Debug>:-Wall;-Wextra;-g;-O0>
              $<$<CONFIG:Release>:-O2;-Wno-format-security>)
  target_compile_definitions(sinen_common INTERFACE $<$<CONFIG:Debug>:DEBUG>
                                                    $<$<CONFIG:Release>:NDEBUG>)
endif()

function(sinen_add_module target)
  add_library(${target} STATIC ${ARGN})
  target_link_libraries(${target} PUBLIC sinen_common)
  set_property(TARGET ${target} PROPERTY CXX_STANDARD 23)
endfunction()

function(sinen_glob_module target module_dir)
  file(GLOB_RECURSE SINEN_MODULE_SOURCES CONFIGURE_DEPENDS
       ${SINEN_SOURCE_DIR}/src/${module_dir}/*.cpp)
  file(GLOB_RECURSE SINEN_MODULE_HEADERS CONFIGURE_DEPENDS
       ${SINEN_SOURCE_DIR}/src/${module_dir}/*.hpp
       ${SINEN_SOURCE_DIR}/src/${module_dir}/*.inl)
  sinen_add_module(${target} ${SINEN_MODULE_SOURCES} ${SINEN_MODULE_HEADERS})
endfunction()

set(SINEN_INTERNAL_MODULES)

macro(sinen_add_enabled_module option directory target)
  if(${option})
    add_subdirectory(${SINEN_SOURCE_DIR}/src/${directory})
    list(APPEND SINEN_INTERNAL_MODULES ${target})
  endif()
endmacro()

sinen_add_enabled_module(SINEN_MODULE_CORE core sinen_core)
sinen_add_enabled_module(SINEN_MODULE_MATH math sinen_math)
sinen_add_enabled_module(SINEN_MODULE_PLATFORM platform sinen_platform)
sinen_add_enabled_module(SINEN_MODULE_AUDIO audio sinen_audio)
sinen_add_enabled_module(SINEN_MODULE_GPU gpu sinen_gpu)
sinen_add_enabled_module(SINEN_MODULE_SHADER_COMPILER shader_compiler
                         sinen_shader_compiler)
sinen_add_enabled_module(SINEN_MODULE_GRAPHICS graphics sinen_graphics)
sinen_add_enabled_module(SINEN_MODULE_COMPUTE compute sinen_compute)
sinen_add_enabled_module(SINEN_MODULE_PHYSICS physics sinen_physics)
sinen_add_enabled_module(SINEN_MODULE_SCRIPT script sinen_script)

if(TARGET sinen_math)
  target_link_libraries(sinen_math PUBLIC sinen_core)
endif()
if(TARGET sinen_platform)
  target_link_libraries(sinen_platform PUBLIC sinen_core sinen_math)
endif()
if(TARGET sinen_gpu)
  target_link_libraries(sinen_gpu PUBLIC sinen_core sinen_math)
  if(TARGET sinen_shader_compiler)
    target_link_libraries(sinen_gpu PUBLIC sinen_shader_compiler)
  endif()
endif()
if(TARGET sinen_graphics)
  target_link_libraries(sinen_graphics PUBLIC sinen_core sinen_math sinen_gpu
                                           sinen_platform)
endif()
if(TARGET sinen_physics)
  target_link_libraries(sinen_physics PUBLIC sinen_core sinen_math)
endif()
if(TARGET sinen_audio)
  target_link_libraries(sinen_audio PUBLIC sinen_core sinen_math sinen_platform)
endif()
if(TARGET sinen_shader_compiler)
  target_link_libraries(sinen_shader_compiler PUBLIC sinen_core sinen_gpu
                                                     sinen_platform)
endif()
if(TARGET sinen_compute)
  target_link_libraries(sinen_compute PUBLIC sinen_core sinen_gpu
                                            sinen_graphics)
endif()
if(TARGET sinen_script)
  target_link_libraries(sinen_script PUBLIC sinen_core sinen_math sinen_platform
                                           sinen_audio sinen_gpu sinen_graphics
                                           sinen_compute sinen_physics
                                           )
endif()

set(SINEN_PLATFORM_SOURCES)
if(WIN32)
  list(APPEND SINEN_PLATFORM_SOURCES ${SINEN_SOURCE_DIR}/docs/icon/icon.rc)
endif()

if(SINEN_BUILD_APP)
  if(BUILD_SHARED_LIBS)
    add_library(sinen SHARED ${SINEN_SOURCE_DIR}/src/main.cpp
                             ${SINEN_PLATFORM_SOURCES})
  else()
    add_executable(sinen ${SINEN_SOURCE_DIR}/src/main.cpp
                         ${SINEN_PLATFORM_SOURCES})
  endif()

  if(EMSCRIPTEN AND NOT BUILD_SHARED_LIBS)
    set_target_properties(sinen PROPERTIES SUFFIX ".html")
  endif()

  target_link_libraries(sinen PRIVATE ${SINEN_INTERNAL_MODULES} SDL3::SDL3)

  set(SINEN_LINK_LIBRARIES)

  if(WIN32)
    list(APPEND SINEN_LINK_LIBRARIES d3d12 dxgi dxguid Ws2_32)
  endif()

  if(ANDROID)
    find_library(ANDROID_LIB android)
    list(APPEND SINEN_LINK_LIBRARIES ${ANDROID_LIB})
  endif()

  target_link_libraries(sinen PUBLIC ${SINEN_LINK_LIBRARIES})

  if(EMSCRIPTEN)
    target_link_options(
      sinen
      PRIVATE
        -sALLOW_MEMORY_GROWTH=1
        -sINITIAL_MEMORY=268435456
        -sMAXIMUM_MEMORY=2147483648)
    if(SINEN_EMSCRIPTEN_PRELOAD_DIR)
      if(NOT IS_DIRECTORY "${SINEN_EMSCRIPTEN_PRELOAD_DIR}")
        message(FATAL_ERROR
                "SINEN_EMSCRIPTEN_PRELOAD_DIR is not a directory: ${SINEN_EMSCRIPTEN_PRELOAD_DIR}")
      endif()
      target_link_options(
        sinen
        PRIVATE
          "SHELL:--preload-file \"${SINEN_EMSCRIPTEN_PRELOAD_DIR}@${SINEN_EMSCRIPTEN_PRELOAD_MOUNT}\"")
    endif()
    if(SINEN_EMSCRIPTEN_EXCEPTIONS)
      target_link_options(sinen PRIVATE -fexceptions
                          -sNO_DISABLE_EXCEPTION_CATCHING=1)
    endif()
    if(SINEN_EMSCRIPTEN_ASSERTIONS)
      target_link_options(sinen PRIVATE -sASSERTIONS=1)
    endif()
    if(SINEN_EMSCRIPTEN_DEBUG_RUNTIME)
      target_link_options(sinen PRIVATE -gsource-map -sASSERTIONS=2
                          -sSAFE_HEAP=1 -sSTACK_OVERFLOW_CHECK=2)
    endif()
  endif()

  if(SINEN_ENABLE_WEBGPU)
    target_copy_webgpu_binaries(sinen)
  endif()

  if(WIN32)
    if(MSVC)
      set(SUBSYSTEM_LINKER_OPTIONS "/SUBSYSTEM:WINDOWS")
    else()
      set(SUBSYSTEM_LINKER_OPTIONS "-mwindows")
    endif()
  endif()

  target_link_options(sinen PRIVATE ${SUBSYSTEM_LINKER_OPTIONS})
  set_property(TARGET sinen PROPERTY CXX_STANDARD 23)
endif()

option(SINEN_BUILD_CPP_TESTS "Build C++ unit tests" ON)
if(SINEN_BUILD_CPP_TESTS)
  if(NOT SINEN_MODULE_CORE)
    message(FATAL_ERROR "SINEN_BUILD_CPP_TESTS=ON requires SINEN_MODULE_CORE=ON")
  endif()
  enable_testing()
  add_subdirectory(${SINEN_SOURCE_DIR}/tests/cpp)
endif()
