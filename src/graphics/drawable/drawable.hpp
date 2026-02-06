#ifndef SINEN_DRAWABLE_WRAPPER_HPP
#define SINEN_DRAWABLE_WRAPPER_HPP
#include <graphics/model/model.hpp>
#include <graphics/texture/texture.hpp>
#include <math/transform/transform.hpp>

namespace sinen {
/**
 * @brief 2D draw object class
 *
 */
struct Draw2D {
public:
  /**
   * @brief Construct a new draw2d object
   *
   */
  Draw2D();
  /**
   * @brief Construct a new draw2d object
   * @param texture Texture to use
   *
   */
  explicit Draw2D(const Texture &texture);
  struct World {
    Vec2 position;
    float rotation;
    Vec2 scale;
  };
  Array<World> worlds;
  Vec2 position;
  float rotation;
  Vec2 scale;
  Model model;
};
/**
 * @brief 3D draw object class
 *
 */
class Draw3D {
public:
  /**
   * @brief Construct a new draw3d object
   *
   */
  Draw3D();
  /**
   * @brief Construct a new draw3d object
   * @param texture Texture to use
   *
   */
  explicit Draw3D(const Texture &texture);
  Vec3 position;
  Vec3 rotation;
  Vec3 scale;
  Array<Transform> worlds;
  const Model &getModel() const;
  Model model;
};
} // namespace sinen
#endif // !SINEN_DRAWABLE_WRAPPER_HPP
