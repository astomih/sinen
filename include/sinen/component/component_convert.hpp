#ifndef COMPONENT_CONVERT_HPP
#define COMPONENT_CONVERT_HPP
#include "components.hpp"
namespace sinen {
class component_convert {
public:
  static draw3d_component &to_draw3d(component &c) {
    return static_cast<draw3d_component &>(c);
  }
};
} // namespace sinen
#endif // COMPONENT_CONVERT_HPP
