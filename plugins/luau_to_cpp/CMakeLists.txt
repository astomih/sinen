set_target_properties(Luau.Ast Luau.Common Luau.VM
                      PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(sinen_luau_to_cpp_converter STATIC source/converter.cpp)
target_link_libraries(sinen_luau_to_cpp_converter PRIVATE Luau.Ast)
set_target_properties(sinen_luau_to_cpp_converter PROPERTIES
                      POSITION_INDEPENDENT_CODE ON)

add_executable(sinen_luau_to_cpp source/cli.cpp)
target_compile_features(sinen_luau_to_cpp PRIVATE cxx_std_17)
target_link_libraries(sinen_luau_to_cpp PRIVATE sinen_luau_to_cpp_converter)
set_target_properties(sinen_luau_to_cpp PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

sinen_add_lua_module(sinen_plugin_luau_to_cpp source/plugin.cpp)
target_link_libraries(sinen_plugin_luau_to_cpp PRIVATE sinen_luau_to_cpp_converter)
set_target_properties(sinen_plugin_luau_to_cpp PROPERTIES
                      OUTPUT_NAME luau_to_cpp DEBUG_POSTFIX ""
                      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
                      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
if(APPLE)
  set_target_properties(sinen_plugin_luau_to_cpp PROPERTIES SUFFIX ".dylib")
endif()
