#ifndef COMPONENT_CONVERT_HPP
#define COMPONENT_CONVERT_HPP
#include "draw3d_component.hpp"
#include "move_component.hpp"
#include "rigidbody_component.hpp"
namespace sinen {
class component_convert {
public:
  static draw3d_component *to_draw3d(component *c) {
    draw3d_component *d3d = dynamic_cast<draw3d_component *>(c);
    if (d3d == nullptr)
      throw std::runtime_error("component_convert::to_draw3d: component is not "
                               "draw3d_component");
    return d3d;
  }
  static move_component *to_move(component *c) {
    move_component *m = dynamic_cast<move_component *>(c);
    if (m == nullptr)
      throw std::runtime_error("component_convert::to_move: component is not "
                               "move_component");
    return m;
  }
  static rigidbody_component *to_rigidbody(component *c) {
    rigidbody_component *r = dynamic_cast<rigidbody_component *>(c);
    if (r == nullptr)
      throw std::runtime_error("component_convert::to_rigidbody: component is "
                               "not rigidbody_component");
    return r;
  }
};
} // namespace sinen
#endif // COMPONENT_CONVERT_HPP
