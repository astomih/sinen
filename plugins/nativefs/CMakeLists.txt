sinen_add_lua_module(
  sinen_plugin_nativefs
  source/nativefs.cpp
  include/nativefs/nativefs.hpp)

target_include_directories(sinen_plugin_nativefs PRIVATE include)
set_target_properties(
  sinen_plugin_nativefs
  PROPERTIES OUTPUT_NAME nativefs
             DEBUG_POSTFIX ""
             RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
             LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
