function(sinen_configure_platform_dependencies)
  if(SINEN_MODULE_GPU AND NOT EMSCRIPTEN)
    find_package(Vulkan REQUIRED)
  endif()

  sinen_configure_platform_dependencies_impl()
endfunction()

function(sinen_configure_platform_dependencies_impl)
endfunction()

function(sinen_configure_platform_webgpu)
  set(SINEN_ENABLE_WEBGPU OFF PARENT_SCOPE)
endfunction()

function(sinen_configure_platform_common target)
  if(NOT MSVC)
    target_compile_options(
      ${target}
      INTERFACE $<$<CONFIG:Debug>:-Wall;-Wextra;-g;-O0>
                $<$<CONFIG:Release>:-O2;-Wno-format-security>)
    target_compile_definitions(${target} INTERFACE $<$<CONFIG:Debug>:DEBUG>
                                                   $<$<CONFIG:Release>:NDEBUG>)
  endif()

  sinen_configure_platform_common_impl(${target})
endfunction()

function(sinen_configure_platform_common_impl target)
endfunction()

function(sinen_configure_platform_gpu target)
endfunction()

function(sinen_configure_platform_shader_compiler target)
endfunction()

function(sinen_collect_platform_sources output_variable)
  set(${output_variable} "" PARENT_SCOPE)
endfunction()

function(sinen_configure_platform_app target)
endfunction()

if(EMSCRIPTEN)
  include(${CMAKE_CURRENT_LIST_DIR}/emscripten.cmake)
elseif(WIN32)
  include(${CMAKE_CURRENT_LIST_DIR}/windows.cmake)
elseif(ANDROID)
  include(${CMAKE_CURRENT_LIST_DIR}/android.cmake)
endif()
