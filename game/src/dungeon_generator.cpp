#include "dungeon_generator.hpp"
#include "corridor.hpp"
#include "room.hpp"

namespace dts {
void dungeon_generator(sinen::grid<int> &grid) {
  if (grid.width() < 3 || grid.height() < 3) {
    return;
  }
  sinen::point2i grid_size{grid.width(), grid.height()};
  int floor = 0;
  int wall = 1;

  int minimum_room_number = 5;
  int maximum_room_number = 20;
  int room_number =
      sinen::random::get_int_range(minimum_room_number, maximum_room_number);
  std::vector<room> rooms;
  sinen::point2i minimum_room_size{10, 10};
  sinen::point2i maximum_room_size{20, 20};
  for (int i = 0; i < room_number; ++i) {
    rooms.emplace_back(grid_size, minimum_room_size, maximum_room_size);
  }
  // fill wall
  for (int y = 0; y < grid.height(); ++y) {
    for (int x = 0; x < grid.width(); ++x) {
      grid[y][x] = wall;
    }
  }
  for (auto &room : rooms) {
    room.fill(grid, floor);
  }

  corridor c;
  c.connect(grid, rooms, floor);

  // Print the grid
  for (int y = 0; y < grid.height(); ++y) {
    for (int x = 0; x < grid.width(); ++x) {
      std::cout << grid[y][x];
    }
    std::cout << std::endl;
  }
}
} // namespace dts