#ifndef SINEN_INSTANCING_WRAPPER_HPP
#define SINEN_INSTANCING_WRAPPER_HPP

#include "../math/vector2.hpp"
#include "../math/vector3.hpp"
#include "../texture/texture.hpp"
#include "../utility/handle_t.hpp"
#include <vector>

namespace sinen {
class draw2d_instancing {
public:
  draw2d_instancing() = default;
  draw2d_instancing(texture texture_handle);
  void draw();
  void add(const vector2 &position, const float &rotation,
           const vector2 &scale);
  void clear();
  struct world {
    vector2 position;
    float rotation;
    vector2 scale;
  };
  std::vector<world> worlds;
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
  void clear();
  struct world {
    vector3 position;
    vector3 rotation;
    vector3 scale;
  };
  std::vector<world> worlds;
  texture texture_handle;
  bool is_draw_depth = true;
  std::string vertex_name = "SPRITE";
};
} // namespace sinen
#endif // !SINEN_INSTANCING_WRAPPER_HPP
