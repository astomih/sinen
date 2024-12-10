cmake_minimum_required(VERSION 3.2)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.12.0.zip
)

set(BUILD_GMOCK CACHE BOOL OFF "" FORCE)
set(INSTALL_GTEST CACHE BOOL OFF "" FORCE)

if(WIN32)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

FetchContent_MakeAvailable(googletest)

project(test CXX)

enable_testing()
enable_language(CXX)

file(GLOB_RECURSE TEST_SOURCES ./*.cpp)

# Unit test
add_executable(unit_test ${TEST_SOURCES})
target_include_directories(unit_test PUBLIC ../include)
target_compile_features(unit_test PUBLIC cxx_std_20)
target_link_libraries(unit_test
  PRIVATE
  GTest::gtest GTest::gtest_main
)

# Add CTest to unit test
include(GoogleTest)
gtest_discover_tests(unit_test)