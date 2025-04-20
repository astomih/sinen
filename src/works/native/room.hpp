#ifndef DTS_ROOM_HPP
#define DTS_ROOM_HPP
#include "glm/ext/vector_int2.hpp"
#include <math/graph/grid.hpp>
#include <math/random.hpp>

namespace dts {
class room {
public:
  room(const glm::ivec2 &grid_size, glm::ivec2 minimum, glm::ivec2 maximum) {
    set_size(minimum, maximum);
    position_randomly(grid_size);
  }

  void fill(sinen::Grid<int> &grid, int floor) const {
    for (int y = 0; y < size.y; ++y) {
      for (int x = 0; x < size.x; ++x) {
        grid[y + position.y][x + position.x] = floor;
      }
    }
  }

  const glm::ivec2 &get_position() const { return position; }
  const glm::ivec2 &get_size() const { return size; }

private:
  void set_size(glm::ivec2 minimum, glm::ivec2 maximum) {
    size.x = sinen::Random::get_int_range(minimum.x, maximum.x);
    size.y = sinen::Random::get_int_range(minimum.y, maximum.y);
  }
  void position_randomly(glm::ivec2 grid_size) {
    position.x = sinen::Random::get_int_range(1, grid_size.x - 1 - size.x);
    position.y = sinen::Random::get_int_range(1, grid_size.y - 1 - size.y);
  }
  glm::ivec2 size;
  glm::ivec2 position;
};

} // namespace dts

#endif // DTS_ROOM_HPP