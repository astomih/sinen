// grid class test

// gtest
#include <gtest/gtest.h>

// grid
#include <sinen/graph/grid.hpp>

namespace size_test {
/**
 * @brief Test the size of the grid
 * @details Test the size of the grid when empty
 */
TEST(SizeGetTest, TestSizeWhenEmpty) {
  sinen::grid<int> grid;
  EXPECT_EQ(grid.size(), 0);
  EXPECT_EQ(grid.width(), 0);
  EXPECT_EQ(grid.height(), 0);
  SUCCEED();
}
/**
 * @brief Test the size of the grid
 * @details Test the size of the grid one element
 */
TEST(SizeGetTest, TestSizeWhenOneElement) {
  sinen::grid<int> grid = {{1}};
  EXPECT_EQ(grid.size(), 1);
  EXPECT_EQ(grid.width(), 1);
  EXPECT_EQ(grid.height(), 1);
  SUCCEED();
}
/**
 * @brief Test the size of the grid
 * @details Test the size of the grid elements
 */
TEST(SizeGetTest, TestSizeWhenElements) {
  sinen::grid<int> grid = {{1, 2}, {1}};
  EXPECT_EQ(grid.size(), 4);
  EXPECT_EQ(grid.width(), 2);
  EXPECT_EQ(grid.height(), 2);
  SUCCEED();
}
} // namespace size_test
namespace resize_test {
/**
 * @brief Test the resize the grid
 * @details Test the resize the grid when empty
 */
TEST(ResizeTest, TestResizeWhenEmpty) {
  sinen::grid<int> grid;
  EXPECT_EQ(grid.size(), 0);
  grid.resize(1, 2);
  EXPECT_EQ(grid.size(), 2);
  EXPECT_EQ(grid.width(), 1);
  EXPECT_EQ(grid.height(), 2);
  grid.clear();
  grid.resize(5, 2);
  EXPECT_EQ(grid.size(), 10);
  EXPECT_EQ(grid.width(), 5);
  EXPECT_EQ(grid.height(), 2);

  SUCCEED();
}
/**
 * @brief Test resize the grid
 * @details Test resize the grid when one element
 */
TEST(ResizeTest, TestResizeWhenOneElement) {
  sinen::grid<int> grid;
  EXPECT_EQ(grid.size(), 0);
  grid.resize(1, 2);
  EXPECT_EQ(grid.size(), 2);
  EXPECT_EQ(grid.width(), 1);
  EXPECT_EQ(grid.height(), 2);
  grid.resize(5, 2);
  EXPECT_EQ(grid.size(), 10);
  EXPECT_EQ(grid.width(), 5);
  EXPECT_EQ(grid.height(), 2);

  SUCCEED();
}
/**
 * @brief Test resize the grid
 * @details Test resize the grid when elements
 */
TEST(ResizeTest, TestResizeWhenElements) {
  sinen::grid<int> grid;
  EXPECT_EQ(grid.size(), 0);
  grid.resize(1, 2);
  EXPECT_EQ(grid.size(), 2);
  EXPECT_EQ(grid.width(), 1);
  EXPECT_EQ(grid.height(), 2);
  grid.resize(5, 2);
  EXPECT_EQ(grid.size(), 10);
  EXPECT_EQ(grid.width(), 5);
  EXPECT_EQ(grid.height(), 2);
  grid.resize(1, 2);
  EXPECT_EQ(grid.size(), 2);
  EXPECT_EQ(grid.width(), 1);
  EXPECT_EQ(grid.height(), 2);

  SUCCEED();
}

} // namespace resize_test