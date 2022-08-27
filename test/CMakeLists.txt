cmake_minimum_required(VERSION 3.2)
project(cmake_example CXX)

find_package(GTest REQUIRED)

enable_testing()
enable_language(CXX)


# Unit test
add_executable(unit_test 
grid_test.cpp)
target_include_directories(unit_test PUBLIC ../include)
target_link_libraries(unit_test
  PRIVATE
        GTest::gtest GTest::gtest_main
  )
# Add CTest to unit test
include(GoogleTest)
gtest_discover_tests(unit_test)