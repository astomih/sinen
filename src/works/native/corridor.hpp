#ifndef DTS_CORRIDOR_HPP
#define DTS_CORRIDOR_HPP
#include "room.hpp"
namespace dts {
class corridor{
public:
  void connect(sinen::Grid<int> &grid, std::vector<room> &rooms,
               float floor_corridor);
  const std::vector<sinen::Point2i> &get_path() { return path; }

 private:
  std::vector<sinen::Point2i> path;
};
} // namespace dts
#endif // DTS_CORRIDOR_HPP
