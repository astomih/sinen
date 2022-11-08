#include "register_script.hpp"
#include <graph/bfs_grid.hpp>
#include <graph/grid.hpp>

namespace sinen {
void register_graph(sol::state &lua) {
  lua["grid"] = [](int width, int height) -> grid<int> {
    return grid<int>(width, height);
  };
  {
    auto v = lua.new_usertype<grid<int>>("", sol::no_construction());
    v["at"] = [](grid<int> &g, int x, int y) { return g.at(x - 1, y - 1); };
    v["width"] = &grid<int>::width;
    v["height"] = &grid<int>::height;
    v["size"] = &grid<int>::size;
    v["clear"] = &grid<int>::clear;
    v["resize"] = &grid<int>::resize;
    v["fill"] = [](grid<int> &g, int value) {
      for (auto &i : g) {
        i = value;
      }
    };
  };
  lua["bfs_grid"] = [](const grid<int> &g) { return bfs_grid(g); };
  {
    auto v = lua.new_usertype<bfs_grid>("", sol::no_construction());
    v["width"] = &bfs_grid::width;
    v["height"] = &bfs_grid::height;
    v["find_path"] = &bfs_grid::find_path;
    v["trace"] = &bfs_grid::trace;
    v["traceable"] = &bfs_grid::traceable;
    v["reset"] = &bfs_grid::reset;
  }
};
} // namespace sinen
