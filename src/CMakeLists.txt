add_library(sinen_common INTERFACE)
target_include_directories(sinen_common INTERFACE ${SINEN_INCLUDE_DIRS})

target_compile_definitions(
  sinen_common
  INTERFACE $<$<CONFIG:Debug>:SINEN_DEBUG_WORKING_DIRECTORY>)

foreach(SINEN_MODULE_NAME CORE MATH PLATFORM AUDIO GPU SHADER_COMPILER GRAPHICS
                          COMPUTE PHYSICS GUI SCRIPT PROFILER)
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
    add_subdirectory(${directory})
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
sinen_add_enabled_module(SINEN_MODULE_GUI gui sinen_gui)
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
if(TARGET sinen_gui)
  target_link_libraries(sinen_gui PUBLIC sinen_core sinen_math sinen_graphics
                                        sinen_platform)
endif()
if(TARGET sinen_script)
  target_link_libraries(sinen_script PUBLIC sinen_core sinen_math sinen_platform
                                           sinen_audio sinen_gpu sinen_graphics
                                           sinen_compute sinen_physics
                                           sinen_gui)
endif()

set(SINEN_PLATFORM_SOURCES)
if(WIN32)
  list(APPEND SINEN_PLATFORM_SOURCES ${SINEN_SOURCE_DIR}/docs/icon/icon.rc)
endif()

if(SINEN_BUILD_APP)
  if(BUILD_SHARED_LIBS)
    add_library(sinen SHARED main.cpp ${SINEN_PLATFORM_SOURCES})
  else()
    add_executable(sinen main.cpp ${SINEN_PLATFORM_SOURCES})
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
