#pragma once
#include <string_view>

#include "../color/color.hpp"
#include "../math/vector2.hpp"
#include "../math/vector3.hpp"

namespace nen {
struct vertex_default_shapes {
  static std::string_view sprite;
  static std::string_view box;
};
struct vertex {
  vector3 position;
  vector3 normal;
  vector2 uv;
  color rgba = nen::color(nen::color(1, 1, 1, 1));
};
} // namespace nen