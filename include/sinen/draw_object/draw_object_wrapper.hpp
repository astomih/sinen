#ifndef SINEN_DRAW_OBJECT_WRAPPER_HPP
#define SINEN_DRAW_OBJECT_WRAPPER_HPP
#include "../math/vector2.hpp"
#include "../math/vector3.hpp"
#include "../texture/texture.hpp"
#include "../utility/handle_t.hpp"
#include "draw_object.hpp"

namespace sinen {
class draw2d {
public:
  draw2d();
  draw2d(texture);
  void draw();
  vector2 position;
  float rotation;
  vector2 scale;
  texture texture_handle;
  std::string vertex_name = "SPRITE";
};
class draw3d {
public:
  draw3d();
  draw3d(texture);
  void draw();
  vector3 position;
  vector3 rotation;
  vector3 scale;
  texture texture_handle;
  bool is_draw_depth = true;
  std::string vertex_name = "SPRITE";
};
} // namespace sinen
#endif // !SINEN_DRAW_OBEJCT_WRAPPER_HPP
