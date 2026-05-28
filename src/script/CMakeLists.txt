sinen_glob_module(sinen_script script)
target_include_directories(
  sinen_script
  PUBLIC ${SINEN_SOURCE_DIR}/src/script/include
         ${SINEN_SOURCE_DIR}/libs/luau-debugger/debugger/include
         ${SINEN_SOURCE_DIR}/libs/cppdap/include)
target_link_libraries(sinen_script PUBLIC Luau.VM Luau.Compiler Luau.Ast
                                          Luau.Require cppdap Luau.Debugger
                                          SDL3::SDL3)
