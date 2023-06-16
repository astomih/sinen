#ifndef DTS_ROOM_HPP
#define DTS_ROOM_HPP
#include <sinen/graph/grid.hpp>
#include <sinen/math/point2.hpp>
#include <sinen/math/random.hpp>

namespace dts {
class room {
public:
  room(const sinen::point2i &grid_size, sinen::point2i minimum,
       sinen::point2i maximum) {
    set_size(minimum, maximum);
    position_randomly(grid_size);
  }

  void fill(sinen::grid<int> &grid, int floor) const {
    for (int y = 0; y < size.y; ++y) {
      for (int x = 0; x < size.x; ++x) {
        grid[y + position.y][x + position.x] = floor;
      }
    }
  }

  const sinen::point2i &get_position() const { return position; }
  const sinen::point2i &get_size() const { return size; }

private:
  void set_size(sinen::point2i minimum, sinen::point2i maximum) {
    size.x = sinen::random::get_int_range(minimum.x, maximum.x);
    size.y = sinen::random::get_int_range(minimum.y, maximum.y);
  }
  void position_randomly(sinen::point2i grid_size) {
    position.x = sinen::random::get_int_range(1, grid_size.x - 1 - size.x);
    position.y = sinen::random::get_int_range(1, grid_size.y - 1 - size.y);
  }
  sinen::point2i size;
  sinen::point2i position;
};

} // namespace dts

#endif // DTS_ROOM_HPP