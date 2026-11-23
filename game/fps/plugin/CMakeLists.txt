set(slab_source "${CMAKE_CURRENT_SOURCE_DIR}/../slab.luau")
set(slab_header "${CMAKE_CURRENT_BINARY_DIR}/slab.generated.hpp")
add_custom_command(
  OUTPUT "${slab_header}"
  COMMAND sinen_luau_to_cpp "${slab_source}" "${slab_header}"
  DEPENDS sinen_luau_to_cpp "${slab_source}"
  VERBATIM)

sinen_add_lua_module(sinen_plugin_fps_collision plugin.cpp "${slab_header}")
target_include_directories(sinen_plugin_fps_collision PRIVATE
                           "${CMAKE_CURRENT_BINARY_DIR}")
set_target_properties(sinen_plugin_fps_collision PROPERTIES
                      OUTPUT_NAME fps_collision DEBUG_POSTFIX "")
if(APPLE)
  set_target_properties(sinen_plugin_fps_collision PROPERTIES SUFFIX ".dylib")
endif()

add_custom_target(sinen_deploy_fps_collision ALL
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "$<TARGET_FILE:sinen_plugin_fps_collision>"
          "${CMAKE_CURRENT_SOURCE_DIR}/../$<TARGET_FILE_NAME:sinen_plugin_fps_collision>"
  DEPENDS sinen_plugin_fps_collision
  VERBATIM)
