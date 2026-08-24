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
