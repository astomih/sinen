if(WIN32)
  set(SLANG_EMBED_CORE_MODULE_SOURCE OFF)
  set(SLANG_ENABLE_EXAMPLES OFF)
  set(SLANG_ENABLE_TESTS OFF)
  set(SLANG_ENABLE_REPLAYER OFF)
  set(SLANG_ENABLE_SLANGC OFF)
  set(SLANG_ENABLE_SLANGD OFF)
  set(SLANG_ENABLE_SLANGI OFF)
  set(SLANG_ENABLE_GFX OFF)
  set(SLANG_RHI_ENABLE OFF)
  set(SLANG_RHI_ENABLE_OPTIX OFF)
  set(SLANG_RHI_ENABLE_CUDA OFF)
  set(SLANG_RHI_ENABLE_NVAPI OFF)
  set(SLANG_RHI_ENABLE_CPU OFF)
  set(SLANG_RHI_ENABLE_D3D11 OFF)
  set(SLANG_RHI_ENABLE_D3D12 OFF)
  set(SLANG_RHI_ENABLE_VULKAN OFF)
  set(SLANG_RHI_ENABLE_WGPU OFF)
  set(SLANG_RHI_ENABLE_AGILITY_SDK OFF)
  set(SLANG_LIB_TYPE STATIC)
endif()

add_subdirectory(libs/slang EXCLUDE_FROM_ALL)
sinen_configure_platform_shader_compiler(slang)

sinen_add_lua_module(
  sinen_plugin_shader_compiler
  source/plugin.cpp
  source/shader_compiler.cpp
  include/shader_compiler/shader_compiler.hpp)

target_include_directories(sinen_plugin_shader_compiler PRIVATE include)
target_compile_definitions(sinen_plugin_shader_compiler
                           PRIVATE SINEN_SHADER_COMPILER_PLUGIN_BUILD)
target_link_libraries(sinen_plugin_shader_compiler PRIVATE slang)
set_target_properties(
  sinen_plugin_shader_compiler
  PROPERTIES OUTPUT_NAME shader_compiler
             DEBUG_POSTFIX ""
             RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
             LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
