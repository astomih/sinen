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

function(sinen_configure_platform_dependencies_impl)
  if(SINEN_EMSCRIPTEN_EXCEPTIONS)
    add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:-fexceptions>")
    add_link_options(-fexceptions -sNO_DISABLE_EXCEPTION_CATCHING=1)
  endif()
endfunction()

function(sinen_configure_platform_webgpu)
  if(NOT SINEN_MODULE_GPU OR TARGET webgpu)
    set(SINEN_ENABLE_WEBGPU OFF PARENT_SCOPE)
    return()
  endif()

  add_library(webgpu INTERFACE)
  target_compile_definitions(webgpu INTERFACE WEBGPU_BACKEND_EMDAWNWEBGPU)
  target_compile_options(webgpu INTERFACE --use-port=emdawnwebgpu)
  target_link_options(webgpu INTERFACE --use-port=emdawnwebgpu)
  if(SINEN_EMSCRIPTEN_ASYNCIFY)
    target_link_options(webgpu INTERFACE -sASYNCIFY=1)
  endif()
  function(target_copy_webgpu_binaries Target)
  endfunction()
  set(SINEN_ENABLE_WEBGPU ON PARENT_SCOPE)
endfunction()

function(sinen_configure_platform_common_impl target)
  target_compile_definitions(${target} INTERFACE EMSCRIPTEN)
  if(SINEN_EMSCRIPTEN_EXCEPTIONS)
    target_compile_options(${target} INTERFACE -fexceptions)
  endif()
  if(SINEN_EMSCRIPTEN_DEBUG_RUNTIME)
    target_compile_options(${target} INTERFACE -gsource-map)
  endif()
endfunction()

function(sinen_configure_platform_app target)
  if(NOT BUILD_SHARED_LIBS)
    set_target_properties(${target} PROPERTIES SUFFIX ".html")
  endif()

  target_link_options(
    ${target}
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
      ${target}
      PRIVATE
        "SHELL:--preload-file \"${SINEN_EMSCRIPTEN_PRELOAD_DIR}@${SINEN_EMSCRIPTEN_PRELOAD_MOUNT}\"")
  endif()
  if(SINEN_EMSCRIPTEN_EXCEPTIONS)
    target_link_options(${target} PRIVATE -fexceptions
                        -sNO_DISABLE_EXCEPTION_CATCHING=1)
  endif()
  if(SINEN_EMSCRIPTEN_ASSERTIONS)
    target_link_options(${target} PRIVATE -sASSERTIONS=1)
  endif()
  if(SINEN_EMSCRIPTEN_DEBUG_RUNTIME)
    target_link_options(${target} PRIVATE -gsource-map -sASSERTIONS=2
                        -sSAFE_HEAP=1 -sSTACK_OVERFLOW_CHECK=2)
  endif()
endfunction()
