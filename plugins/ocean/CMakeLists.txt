add_library(sinen_ocean STATIC source/ocean.cpp include/ocean/ocean.hpp)
target_include_directories(sinen_ocean PUBLIC include)
target_compile_features(sinen_ocean PUBLIC cxx_std_20)
set_target_properties(sinen_ocean PROPERTIES POSITION_INDEPENDENT_CODE ON)

sinen_add_lua_module(sinen_plugin_ocean source/plugin.cpp)
target_link_libraries(sinen_plugin_ocean PRIVATE sinen_ocean)
set_target_properties(sinen_plugin_ocean PROPERTIES
  OUTPUT_NAME ocean DEBUG_POSTFIX ""
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
