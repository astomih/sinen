set(SINEN_ENABLE_NATIVE_WEBGPU_DAWN_DEFAULT ON)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(SINEN_ENABLE_NATIVE_WEBGPU_DAWN_DEFAULT OFF)
endif()
option(SINEN_ENABLE_NATIVE_WEBGPU_DAWN
       "Enable native WebGPU backend through Dawn"
       ${SINEN_ENABLE_NATIVE_WEBGPU_DAWN_DEFAULT})
unset(SINEN_ENABLE_NATIVE_WEBGPU_DAWN_DEFAULT)

function(sinen_configure_platform_webgpu)
  if(NOT SINEN_MODULE_GPU OR NOT SINEN_ENABLE_NATIVE_WEBGPU_DAWN OR
     TARGET webgpu)
    set(SINEN_ENABLE_WEBGPU OFF PARENT_SCOPE)
    return()
  endif()

  set(WEBGPU_BACKEND DAWN CACHE STRING "Native WebGPU backend" FORCE)
  set(WEBGPU_LINK_TYPE SHARED CACHE STRING "Native WebGPU link type" FORCE)
  set(WEBGPU_BUILD_FROM_SOURCE OFF CACHE BOOL
      "Build WebGPU backend from source" FORCE)
  set(SINEN_PREVIOUS_CMAKE_FOLDER ${CMAKE_FOLDER})
  set(CMAKE_FOLDER "webgpu")
  add_subdirectory(${SINEN_SOURCE_DIR}/libs/WebGPU-distribution
                   EXCLUDE_FROM_ALL)
  set(CMAKE_FOLDER ${SINEN_PREVIOUS_CMAKE_FOLDER})
  set(SINEN_ENABLE_WEBGPU ON PARENT_SCOPE)
endfunction()

function(sinen_configure_platform_common_impl target)
  if(NOT MSVC)
    return()
  endif()

  target_compile_options(${target} INTERFACE /bigobj)
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
    target_include_directories(${target} SYSTEM
                               INTERFACE ${SINEN_MSVC_SYSTEM_INCLUDES})
  endif()
endfunction()

function(sinen_configure_platform_gpu target)
  target_link_libraries(${target} PUBLIC d3d12 dxgi dxguid)
endfunction()

function(sinen_configure_platform_shader_compiler target)
  if(MSVC AND TARGET libcmark-gfm)
    target_compile_options(
      libcmark-gfm
      PUBLIC $<$<COMPILE_LANG_AND_ID:C,MSVC>:/utf-8>
             $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/utf-8>)
  endif()
endfunction()

function(sinen_collect_platform_sources output_variable)
  set(${output_variable} ${SINEN_SOURCE_DIR}/docs/icon/icon.rc PARENT_SCOPE)
endfunction()

function(sinen_configure_platform_app target)
  target_link_libraries(${target} PUBLIC d3d12 dxgi dxguid Ws2_32)
  if(MSVC)
    target_link_options(${target} PRIVATE /SUBSYSTEM:WINDOWS)
  else()
    target_link_options(${target} PRIVATE -mwindows)
  endif()
endfunction()
