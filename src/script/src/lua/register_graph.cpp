/*
#include "glm/ext/vector_int2.hpp"
#include "register_script.hpp"
#include <math/graph/bfs_grid.hpp>
#include <math/graph/grid.hpp>

namespace sinen {
void register_graph(sol::state &lua) {
  lua["Grid"] = [](int width, int height) -> Grid<int> {
    return Grid<int>(width, height);
  };
  {
    auto v = lua.new_usertype<Grid<int>>("", sol::no_construction());
    v["at"] = [](Grid<int> &g, int x, int y) { return g.at(x - 1, y - 1); };
    v["set"] = [](Grid<int> &g, int x, int y, int v) {
      return g.at(x - 1, y - 1) = v;
    };
    v["width"] = &Grid<int>::width;
    v["height"] = &Grid<int>::height;
    v["size"] = &Grid<int>::size;
    v["clear"] = &Grid<int>::clear;
    v["resize"] = &Grid<int>::resize;
    v["fill"] = [](Grid<int> &g, int value) {
      for (auto &i : g) {
        i = value;
      }
    };
  };
  lua["BFSGrid"] = [](const Grid<int> &g) { return BFSGrid(g); };
  {
    auto v = lua.new_usertype<BFSGrid>("", sol::no_construction());
    v["width"] = &BFSGrid::width;
    v["height"] = &BFSGrid::height;
    v["find_path"] = [](BFSGrid &g, const glm::ivec2 &start,
                        const glm::ivec2 &end) {
      return g.find_path({start.x - 1, start.y - 1}, {end.x - 1, end.y - 1});
    };
    v["trace"] = [](BFSGrid &g) {
      auto t = g.trace();
      return glm::ivec2{t.x + 1, t.y + 1};
    };
    v["traceable"] = &BFSGrid::traceable;
    v["reset"] = &BFSGrid::reset;
  }
};
} // namespace sinen

*/