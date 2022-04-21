#pragma once
#include "../math/matrix4.hpp"
#include "../math/vector3.hpp"

namespace nen {
class camera {
public:
  camera();
  void update();
  matrix4 get_view() const { return view; }
  matrix4 get_projection() const { return projection; }
  vector3 position;
  vector3 target;
  vector3 up;
  float fov;
  float aspect;
  float near;
  float far;

private:
  matrix4 view;
  matrix4 projection;
};
} // namespace nen