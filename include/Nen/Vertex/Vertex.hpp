#pragma once
#include "../Color/Color.hpp"
#include "../Math/Vector2.hpp"
#include "../Math/Vector3.hpp"
#include "Color/Color.hpp"
#include "Math/Vector3.hpp"

namespace nen {
struct vertex {
  vector3 position;
  vector3 normal;
  vector2 uv;
  color color = nen::color(nen::color(1, 1, 1, 1));
};
} // namespace nen