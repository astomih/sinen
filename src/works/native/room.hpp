#ifndef DTS_ROOM_HPP
#define DTS_ROOM_HPP
#include <sinen/graph/grid.hpp>
#include <sinen/math/point2.hpp>
#include <sinen/math/random.hpp>

namespace dts {
class room {
public:
  room(const sinen::Point2i &grid_size, sinen::Point2i minimum,
       sinen::Point2i maximum) {
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

  const sinen::Point2i &get_position() const { return position; }
  const sinen::Point2i &get_size() const { return size; }

private:
  void set_size(sinen::Point2i minimum, sinen::Point2i maximum) {
    size.x = sinen::Random::get_int_range(minimum.x, maximum.x);
    size.y = sinen::Random::get_int_range(minimum.y, maximum.y);
  }
  void position_randomly(sinen::Point2i grid_size) {
    position.x = sinen::Random::get_int_range(1, grid_size.x - 1 - size.x);
    position.y = sinen::Random::get_int_range(1, grid_size.y - 1 - size.y);
  }
  sinen::Point2i size;
  sinen::Point2i position;
};

} // namespace dts

#endif // DTS_ROOM_HPP