#ifndef SINEN_DRAW_OBJECT_WRAPPER_HPP
#define SINEN_DRAW_OBJECT_WRAPPER_HPP
#include "../math/vector2.hpp"
#include "../math/vector3.hpp"
#include "../texture/texture.hpp"
#include "../utility/handle_t.hpp"
#include "drawable.hpp"

namespace sinen {
/**
 * @brief 2D draw object class
 *
 */
struct draw2d {
public:
  /**
   * @brief Construct a new draw2d object
   *
   */
  draw2d();
  /**
   * @brief Construct a new draw2d object
   * @param texture Texture to use
   *
   */
  draw2d(texture);
  /**
   * @brief Draw object
   *
   */
  void draw();
  vector2 position;
  float rotation;
  vector2 scale;
  texture texture_handle;
  std::string vertex_name = "SPRITE";
  std::shared_ptr<drawable> obj;
};
/**
 * @brief 3D draw object class
 *
 */
class draw3d {
public:
  /**
   * @brief Construct a new draw3d object
   *
   */
  draw3d();
  /**
   * @brief Construct a new draw3d object
   * @param texture Texture to use
   *
   */
  draw3d(texture);
  /**
   * @brief Draw object
   *
   */
  void draw();
  vector3 position;
  vector3 rotation;
  vector3 scale;
  texture texture_handle;
  bool is_draw_depth = true;
  std::string vertex_name = "SPRITE";
  std::shared_ptr<drawable> obj;
};
} // namespace sinen
#endif // !SINEN_DRAW_OBEJCT_WRAPPER_HPP
