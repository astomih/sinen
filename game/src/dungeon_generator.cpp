#include "dungeon_generator.hpp"

namespace dts {
void dungeon_generator(sinen::grid<int> &grid) {
  constexpr int floor = 0;
  constexpr int wall = 1;
  if (grid.width() < 3 || grid.height() < 3) {
    return;
  }
  for (int x = 0; x < grid.width(); ++x) {
    grid[0][x] = 1;
    grid[grid.height() - 1][x] = 1;
  }
  for (int y = 0; y < grid.height(); ++y) {
    grid[y][0] = 1;
    grid[y][grid.width() - 1] = 1;
  }
}
} // namespace dts