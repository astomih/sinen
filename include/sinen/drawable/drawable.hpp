#ifndef SINEN_DRAWABLE_HPP
#define SINEN_DRAWABLE_HPP
#include "../shader/shader.hpp"
#include "../texture/texture.hpp"
#include "instance_data.hpp"

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
  /**
   * @brief World matrix to instancing data
   *
   * @param mat input world matrix
   * @param data output instancing data
   */
  void world_to_instance_data(const matrix4 &mat, instance_data &data);
  /**
   * @brief instance size
   *
   */
  std::size_t size;
  /**
   * @brief instance data
   *
   */
  std::vector<instance_data> data;
};

} // namespace sinen
#endif // !SINEN_DRAWABLE_HPP
