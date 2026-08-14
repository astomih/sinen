add_subdirectory(googletest EXCLUDE_FROM_ALL)

add_executable(sinen_allocator_tests allocator_tests.cpp)
target_link_libraries(sinen_allocator_tests PRIVATE sinen_core GTest::gtest_main)
set_property(TARGET sinen_allocator_tests PROPERTY CXX_STANDARD 23)

add_test(NAME sinen_allocator_tests COMMAND sinen_allocator_tests)

add_executable(sinen_license_report_tests license_report_tests.cpp)
target_link_libraries(sinen_license_report_tests
                      PRIVATE sinen_core GTest::gtest_main)
set_property(TARGET sinen_license_report_tests PROPERTY CXX_STANDARD 23)

add_test(NAME sinen_license_report_tests COMMAND sinen_license_report_tests)

if(TARGET sinen_gpu)
  add_executable(sinen_shader_bundle_tests shader_bundle_tests.cpp)
  target_link_libraries(sinen_shader_bundle_tests
                        PRIVATE sinen_gpu GTest::gtest_main)
  target_compile_definitions(
    sinen_shader_bundle_tests
    PRIVATE SINEN_TEST_SOURCE_DIR="${SINEN_SOURCE_DIR}")
  set_property(TARGET sinen_shader_bundle_tests PROPERTY CXX_STANDARD 23)

  add_test(NAME sinen_shader_bundle_tests COMMAND sinen_shader_bundle_tests)
endif()

if(TARGET sinen_script)
  if(WIN32 OR EMSCRIPTEN)
    sinen_add_lua_module(sinen_test_native_module native_module_fixture.cpp)
  endif()
  if(WIN32)
    sinen_add_lua_module(sinen_test_failing_native_module
                         native_module_failure_fixture.cpp)
  endif()

  add_executable(
    sinen_luau_binding_tests
    luau_binding_tests.cpp
    ${SINEN_SOURCE_DIR}/src/script/bindings/binding_registration.cpp)
  if(WIN32)
    target_sources(
      sinen_luau_binding_tests
      PRIVATE native_module_tests.cpp
              ${SINEN_SOURCE_DIR}/src/script/native_module.cpp)
    target_compile_definitions(
      sinen_luau_binding_tests
      PRIVATE
        SINEN_TEST_NATIVE_MODULE_PATH="$<TARGET_FILE:sinen_test_native_module>")
    target_compile_definitions(
      sinen_luau_binding_tests
      PRIVATE
        SINEN_TEST_FAILING_NATIVE_MODULE_PATH="$<TARGET_FILE:sinen_test_failing_native_module>")
    add_dependencies(sinen_luau_binding_tests sinen_test_native_module
                     sinen_test_failing_native_module)
  endif()
  target_include_directories(sinen_luau_binding_tests
                             PRIVATE ${SINEN_SOURCE_DIR}/src
                                     ${SINEN_SOURCE_DIR}/include/sinen)
  target_link_libraries(sinen_luau_binding_tests
                        PRIVATE Luau.VM Luau.Compiler GTest::gtest_main)
  set_property(TARGET sinen_luau_binding_tests PROPERTY CXX_STANDARD 23)

  add_test(NAME sinen_luau_binding_tests COMMAND sinen_luau_binding_tests)
  set_tests_properties(sinen_luau_binding_tests
                       PROPERTIES WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endif()
