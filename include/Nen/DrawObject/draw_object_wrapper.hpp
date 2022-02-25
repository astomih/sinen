#pragma once
#include "../Math/Vector2.hpp"
#include "../Math/Vector3.hpp"
#include "../Utility/handle_t.hpp"

namespace nen {
struct draw2d_object {
  vector2 position;
  float rotation;
  vector2 scale;
  handle_t texture_handle;
};
struct draw3d_object {
  vector3 position;
  vector3 rotation;
  vector3 scale;
  handle_t texture_handle;
};
} // namespace nen