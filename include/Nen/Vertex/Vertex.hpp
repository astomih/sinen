#pragma once
#include "../Color/Color.hpp"
#include "../Math/Vector2.hpp"
#include "../Math/Vector3.hpp"
#include "Color/Color.hpp"
#include "Math/Vector3.hpp"
#include <string_view>

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