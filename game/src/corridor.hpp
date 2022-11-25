#ifndef DTS_CORRIDOR_HPP
#define DTS_CORRIDOR_HPP
#include "room.hpp"
namespace dts {
class corridor {
public:
  void connect(sinen::grid<int> &grid, std::vector<room> &rooms,
               float floor_corridor);
  const std::vector<sinen::point2i> &get_path() { return path; }

 private:
  std::vector<sinen::point2i> path;
};
} // namespace dts
#endif // DTS_CORRIDOR_HPP
