#ifndef SINEN_DRAWABLE_HPP
#define SINEN_DRAWABLE_HPP
#include "../math/vector2.hpp"
#include "../shader/shader.hpp"
#include "../texture/texture.hpp"

namespace sinen {
/**
 * @brief Drawable object class
 *
 */
struct drawable {
  /**
   * @brief Construct a new draw object object
   *
   */
  drawable();
  /**
   * @brief Destroy the draw object object
   *
   */
  ~drawable();
  /**
   * @brief Shader parameter
   *
   */
  struct parameter {
    parameter() {
      float data[16] = {};
      user = matrix4(data);
    }
    matrix4 world;
    matrix4 view;
    matrix4 proj;
    matrix4 user;
  };
  // parameter
  parameter param;
  /**
   * @brief Shader data
   *
   */
  shader shade;
  /**
   * @brief Use texture
   *
   */
  texture binding_texture;
  /**
   * @brief Vertex index
   *
   */
  std::string vertexIndex;
  int drawOrder = 100;
};

} // namespace sinen
#endif // !SINEN_DRAWABLE_HPP
