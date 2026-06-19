cmake_minimum_required(VERSION 3.20)
include(CMakePrintHelpers)
project(sinen)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(SINEN_SOURCE_DIR ${CMAKE_SOURCE_DIR})
include(${SINEN_SOURCE_DIR}/cmake/platform.cmake)

option(SINEN_BUILD_APP "Build the Sinen application executable" ON)
option(SINEN_MODULE_CORE "Build the Core module" ON)
option(SINEN_MODULE_MATH "Build the Math module" ON)
option(SINEN_MODULE_PLATFORM "Build the Platform module" ON)
option(SINEN_MODULE_AUDIO "Build the Audio module" ON)
option(SINEN_MODULE_GPU "Build the GPU module" ON)
option(SINEN_MODULE_SHADER_COMPILER "Build the ShaderCompiler module" ON)
option(SINEN_MODULE_GRAPHICS "Build the Graphics module" ON)
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
sinen_require_module(PHYSICS CORE)
sinen_require_module(PHYSICS MATH)
sinen_require_module(SCRIPT CORE)
sinen_require_module(SCRIPT MATH)
sinen_require_module(SCRIPT PLATFORM)
sinen_require_module(SCRIPT AUDIO)
sinen_require_module(SCRIPT GPU)
sinen_require_module(SCRIPT GRAPHICS)
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

sinen_configure_platform_dependencies()

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

sinen_configure_platform_webgpu()

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
  sinen_configure_platform_shader_compiler(slang)
endif()

add_library(sinen_common INTERFACE)
target_include_directories(sinen_common INTERFACE ${SINEN_INCLUDE_DIRS})

target_compile_definitions(
  sinen_common
  INTERFACE $<$<CONFIG:Debug>:SINEN_DEBUG_WORKING_DIRECTORY>)

foreach(SINEN_MODULE_NAME CORE MATH PLATFORM AUDIO GPU SHADER_COMPILER GRAPHICS
                          PHYSICS SCRIPT PROFILER)
  if(SINEN_MODULE_${SINEN_MODULE_NAME})
    target_compile_definitions(
      sinen_common INTERFACE SINEN_MODULE_${SINEN_MODULE_NAME})
  endif()
endforeach()

if(SINEN_ENABLE_WEBGPU)
  target_compile_definitions(sinen_common INTERFACE SINEN_ENABLE_WEBGPU)
endif()

sinen_configure_platform_common(sinen_common)

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
       ${SINEN_SOURCE_DIR}/src/${module_dir}/*.inl
       ${SINEN_SOURCE_DIR}/include/${module_dir}/*.hpp
       ${SINEN_SOURCE_DIR}/include/${module_dir}/*.inl)
  sinen_add_module(${target} ${SINEN_MODULE_SOURCES} ${SINEN_MODULE_HEADERS})
endfunction()

set(SINEN_INTERNAL_MODULES)

function(sinen_add_core_module)
  sinen_glob_module(sinen_core core)
  target_sources(sinen_core PRIVATE ${SINEN_SOURCE_DIR}/libs/tlsf/tlsf.c)
  target_include_directories(
    sinen_core
    PUBLIC ${SINEN_SOURCE_DIR}/libs/rapidjson/include
           ${SINEN_SOURCE_DIR}/libs/tlsf)
  target_link_libraries(sinen_core PUBLIC SDL3::SDL3)
  if(SINEN_MODULE_PROFILER)
    target_link_libraries(sinen_core PUBLIC Tracy::TracyClient)
  endif()
endfunction()

function(sinen_add_math_module)
  sinen_glob_module(sinen_math math)
endfunction()

function(sinen_add_platform_module)
  sinen_glob_module(sinen_platform platform)
  target_sources(sinen_platform PRIVATE
                 ${SINEN_SOURCE_DIR}/libs/tinyexr/deps/miniz/miniz.c)
  target_include_directories(sinen_platform
                             PUBLIC ${SINEN_SOURCE_DIR}/libs/tinyexr/deps/miniz)
  target_link_libraries(sinen_platform PUBLIC SDL3::SDL3)
endfunction()

function(sinen_add_audio_module)
  sinen_glob_module(sinen_audio audio)
  target_include_directories(sinen_audio PUBLIC ${SINEN_SOURCE_DIR}/libs/miniaudio)
  target_link_libraries(sinen_audio PUBLIC SDL3::SDL3)
endfunction()

function(sinen_add_gpu_module)
  file(GLOB_RECURSE SINEN_GPU_SOURCES CONFIGURE_DEPENDS
       ${SINEN_SOURCE_DIR}/src/gpu/*.cpp)
  file(GLOB_RECURSE SINEN_GPU_HEADERS CONFIGURE_DEPENDS
       ${SINEN_SOURCE_DIR}/src/gpu/*.hpp ${SINEN_SOURCE_DIR}/src/gpu/*.inl
       ${SINEN_SOURCE_DIR}/include/gpu/*.hpp
       ${SINEN_SOURCE_DIR}/include/gpu/*.inl)

  if(NOT SINEN_ENABLE_WEBGPU)
    list(FILTER SINEN_GPU_SOURCES EXCLUDE REGEX
         ".*[\\\\/]src[\\\\/]gpu[\\\\/]backend[\\\\/]webgpu[\\\\/].*\\.cpp$")
    list(FILTER SINEN_GPU_HEADERS EXCLUDE REGEX
         ".*[\\\\/]src[\\\\/]gpu[\\\\/]backend[\\\\/]webgpu[\\\\/].*\\.(hpp|inl)$")
  endif()

  sinen_add_module(sinen_gpu ${SINEN_GPU_SOURCES} ${SINEN_GPU_HEADERS})
  target_include_directories(
    sinen_gpu
    PUBLIC ${Vulkan_INCLUDE_DIR} ${SINEN_SOURCE_DIR}/libs/volk
           ${SINEN_SOURCE_DIR}/libs/VulkanMemoryAllocator/include)
  target_link_libraries(sinen_gpu PUBLIC SDL3::SDL3)

  if(SINEN_ENABLE_WEBGPU)
    target_link_libraries(sinen_gpu PUBLIC webgpu)
    if(TARGET dawn::webgpu_dawn)
      get_target_property(SINEN_DAWN_INCLUDE_DIRS dawn::webgpu_dawn
                          INTERFACE_INCLUDE_DIRECTORIES)
      if(SINEN_DAWN_INCLUDE_DIRS)
        target_include_directories(sinen_gpu BEFORE
                                   PUBLIC ${SINEN_DAWN_INCLUDE_DIRS})
      endif()
    endif()
  endif()

  sinen_configure_platform_gpu(sinen_gpu)
endfunction()

function(sinen_add_shader_compiler_module)
  sinen_glob_module(sinen_shader_compiler shader_compiler)
  target_link_libraries(sinen_shader_compiler PUBLIC slang)
endfunction()

function(sinen_add_graphics_module)
  sinen_glob_module(sinen_graphics graphics)
  target_include_directories(
    sinen_graphics
    PUBLIC ${SINEN_SOURCE_DIR}/libs/stb ${SINEN_SOURCE_DIR}/libs/tinyexr
           ${SINEN_SOURCE_DIR}/libs/tinyexr/deps/miniz)
  target_link_libraries(sinen_graphics PUBLIC SDL3::SDL3 assimp meshoptimizer
                                              ktx_read msdfgen-core)
endfunction()

function(sinen_add_physics_module)
  sinen_glob_module(sinen_physics physics)
  target_link_libraries(sinen_physics PUBLIC Jolt)
endfunction()

function(sinen_add_script_module)
  sinen_glob_module(sinen_script script)
  target_include_directories(
    sinen_script
    PUBLIC ${SINEN_SOURCE_DIR}/src/script/include
           ${SINEN_SOURCE_DIR}/libs/luau-debugger/debugger/include
           ${SINEN_SOURCE_DIR}/libs/cppdap/include)
  target_link_libraries(sinen_script PUBLIC Luau.VM Luau.Compiler Luau.Ast
                                            Luau.Require cppdap Luau.Debugger
                                            SDL3::SDL3)
endfunction()

macro(sinen_add_enabled_module option module target)
  if(${option})
    cmake_language(CALL sinen_add_${module}_module)
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
if(TARGET sinen_script)
  target_link_libraries(sinen_script PUBLIC sinen_core sinen_math sinen_platform
                                           sinen_audio sinen_gpu sinen_graphics
                                           sinen_physics
                                           )
endif()

sinen_collect_platform_sources(SINEN_PLATFORM_SOURCES)

if(SINEN_BUILD_APP)
  if(BUILD_SHARED_LIBS)
    add_library(sinen SHARED ${SINEN_SOURCE_DIR}/src/main.cpp
                             ${SINEN_PLATFORM_SOURCES})
  else()
    add_executable(sinen ${SINEN_SOURCE_DIR}/src/main.cpp
                         ${SINEN_PLATFORM_SOURCES})
  endif()

  target_link_libraries(sinen PRIVATE ${SINEN_INTERNAL_MODULES} SDL3::SDL3)
  target_include_directories(sinen PRIVATE ${SINEN_SOURCE_DIR}/src)
  sinen_configure_platform_app(sinen)

  if(SINEN_ENABLE_WEBGPU)
    target_copy_webgpu_binaries(sinen)
  endif()

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
