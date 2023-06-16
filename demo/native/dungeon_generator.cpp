#include "dungeon_generator.hpp"
#include "corridor.hpp"
#include "room.hpp"
#include <sinen/graph/bfs_grid.hpp>
#include <sinen/script/script.hpp>
#include <sol/sol.hpp>

namespace dts {
void dungeon_generator(sinen::grid<int> &grid) {
  bool recreate = false;
  {
    if (grid.width() < 3 || grid.height() < 3) {
      return;
    }
    sinen::point2i grid_size{grid.width(), grid.height()};
    sol::state &lua = *(sol::state *)sinen::script::get_state();
    auto map_chip = lua["MAP_CHIP"];
    int floor = map_chip["FLOOR"];
    int wall = map_chip["WALL"];
    int player = map_chip["PLAYER"];
    int key = map_chip["KEY"];
    int stair = map_chip["STAIR"];

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
    grid[stairs.y][stairs.x] = stair;
    // Set the key position
    sinen::point2i keys = rooms[rooms.size() / 2].get_position();
    keys.x += rooms[rooms.size() / 2].get_size().x / 2;
    keys.y += rooms[rooms.size() / 2].get_size().y / 2;
    grid[keys.y][keys.x] = key;
    // Set the player start position
    sinen::point2i player_start = rooms[0].get_position();
    player_start.x += rooms[0].get_size().x / 2;
    player_start.y += rooms[0].get_size().y / 2;
    grid[player_start.y][player_start.x] = player;
    ///////////////////////////////////////////////////////

    corridor c;
    c.connect(grid, rooms, floor);
    {
      sinen::bfs_grid bfs(grid);
      if (!bfs.find_path(player_start, keys)) {
        recreate = true;
      }
    }
    {
      sinen::bfs_grid bfs(grid);
      if (!bfs.find_path(keys, stairs)) {
        recreate = true;
      }
    }
    {
      sinen::bfs_grid bfs(grid);
      if (!bfs.find_path(player_start, stairs)) {
        recreate = true;
      }
    }
  }
  if (recreate) {
    dungeon_generator(grid);
  }
}
} // namespace dts