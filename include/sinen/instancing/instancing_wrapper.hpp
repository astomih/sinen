#pragma once
#include "../math/vector2.hpp"
#include "../math/vector3.hpp"
#include "../texture/texture.hpp"
#include "../utility/handle_t.hpp"
#include <vector>

namespace nen {
class draw2d_instancing {
public:
  draw2d_instancing() = default;
  draw2d_instancing(texture texture_handle);
  void draw();
  void add(const vector2 &position, const float &rotation,
           const vector2 &scale);
  std::vector<vector2> position;
  std::vector<float> rotation;
  std::vector<vector2> scale;
  texture texture_handle;
  std::string vertex_name = "SPRITE";
};
class draw3d_instancing {
public:
  draw3d_instancing() = default;
  draw3d_instancing(texture texture_handle);

  void draw();
  void add(const vector3 &position, const vector3 &rotation,
           const vector3 &scale);
  std::vector<vector3> position;
  std::vector<vector3> rotation;
  std::vector<vector3> scale;
  texture texture_handle;
  std::string vertex_name = "SPRITE";
};
} // namespace nen