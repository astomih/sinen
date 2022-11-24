#ifndef DTS_CORRIDOR_HPP
#define DTS_CORRIDOR_HPP
#include "room.hpp"
namespace dts {
class corridor {
public:
  void connect(sinen::grid<int> &grid, std::vector<room> &rooms,
               float floor_corridor);
};
} // namespace dts
#endif // DTS_CORRIDOR_HPP