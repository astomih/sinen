#ifndef DTS_CORRIDOR_HPP
#define DTS_CORRIDOR_HPP
#include "glm/ext/vector_int2.hpp"
#include "room.hpp"
namespace dts {
class corridor {
public:
  void connect(sinen::Grid<int> &grid, std::vector<room> &rooms,
               float floor_corridor);
  const std::vector<glm::ivec2> &get_path() { return path; }

private:
  std::vector<glm::ivec2> path;
};
} // namespace dts
#endif // DTS_CORRIDOR_HPP
