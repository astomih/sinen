add_subdirectory(googletest EXCLUDE_FROM_ALL)

add_executable(sinen_allocator_tests allocator_tests.cpp)
target_link_libraries(sinen_allocator_tests PRIVATE sinen_core GTest::gtest_main)
set_property(TARGET sinen_allocator_tests PROPERTY CXX_STANDARD 23)

add_test(NAME sinen_allocator_tests COMMAND sinen_allocator_tests)

if(TARGET sinen_script)
  add_executable(
    sinen_luau_binding_tests
    luau_binding_tests.cpp
    ${SINEN_SOURCE_DIR}/src/script/bindings/binding_registration.cpp)
  target_include_directories(sinen_luau_binding_tests
                             PRIVATE ${SINEN_SOURCE_DIR}/src)
  target_link_libraries(sinen_luau_binding_tests
                        PRIVATE LuaBridge Luau.VM Luau.Compiler
                                GTest::gtest_main)
  set_property(TARGET sinen_luau_binding_tests PROPERTY CXX_STANDARD 23)

  add_test(NAME sinen_luau_binding_tests COMMAND sinen_luau_binding_tests)
  set_tests_properties(sinen_luau_binding_tests
                       PROPERTIES WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endif()
