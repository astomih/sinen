#include "dungeon_generator.hpp"
#include "corridor.hpp"
#include "room.hpp"
#include <sinen/graph/bfs_grid.hpp>

namespace dts {
void dungeon_generator(sinen::grid<int> &grid) {
  if (grid.width() < 3 || grid.height() < 3) {
    return;
  }
  sinen::point2i grid_size{grid.width(), grid.height()};
  int floor = 0;
  int wall = 1;

  int minimum_room_number = 5;
  int maximum_room_number = 14;
  int room_number =
      sinen::random::get_int_range(minimum_room_number, maximum_room_number);
  std::vector<room> rooms;
  sinen::point2i minimum_room_size{5, 5};
  sinen::point2i maximum_room_size{5 + maximum_room_number - room_number,
                                   5 + maximum_room_number - room_number};
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

  // rooms sort by x
  std::sort(rooms.begin(), rooms.end(), [](const room &a, const room &b) {
    return a.get_position().x < b.get_position().x;
  });
  // Set room interior
  ///////////////////////////////////////////////////////
  // Set the stairs position
  sinen::point2i stairs = rooms[rooms.size() - 1].get_position();
  stairs.x += rooms[rooms.size() - 1].get_size().x / 2;
  stairs.y += rooms[rooms.size() - 1].get_size().y / 2;
  grid[stairs.y][stairs.x] = 2;
  // Set the key position
  sinen::point2i key = rooms[rooms.size() / 2].get_position();
  key.x += rooms[rooms.size() / 2].get_size().x / 2;
  key.y += rooms[rooms.size() / 2].get_size().y / 2;
  grid[key.y][key.x] = 3;
  // Set the player start position
  sinen::point2i player_start = rooms[0].get_position();
  player_start.x += rooms[0].get_size().x / 2;
  player_start.y += rooms[0].get_size().y / 2;
  grid[player_start.y][player_start.x] = 4;
  ///////////////////////////////////////////////////////

  corridor c;
  c.connect(grid, rooms, floor);
  sinen::bfs_grid bfs(grid);
  if (!bfs.find_path(player_start, stairs)) {
    dungeon_generator(grid);
  }
  // print grid
  for (int y = 0; y < grid.height(); ++y) {
    for (int x = 0; x < grid.width(); ++x) {
      std::cout << grid[y][x];
    }
    std::cout << std::endl;
  }
}
} // namespace dts