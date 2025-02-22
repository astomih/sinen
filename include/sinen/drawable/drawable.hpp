#ifndef SINEN_DRAWABLE_HPP
#define SINEN_DRAWABLE_HPP
#include "../model/model.hpp"
#include "../shader/shader.hpp"
#include "../texture/texture.hpp"
#include "instance_data.hpp"


namespace sinen {
/**
 * @brief Drawable object class
 *
 */
struct Drawable {
  /**
   * @brief Construct a new draw object object
   *
   */
  Drawable();
  /**
   * @brief Destroy the draw object object
   *
   */
  ~Drawable();
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
    matrix4 light_view;
    matrix4 light_proj;
  };
  // parameter
  parameter param;
  /**
   * @brief Shader data
   *
   */
  Shader shade;
  /**
   * @brief Use texture
   *
   */
  Texture binding_texture;
  Model model;
  int drawOrder = 100;
  /**
   * @brief World matrix to instancing data
   *
   * @param mat input world matrix
   * @param data output instancing data
   */
  void world_to_instance_data(const matrix4 &mat, InstanceData &data);
  /**
   * @brief instance size
   *
   */
  std::size_t size() { return sizeof(InstanceData) * data.size(); }
  /**
   * @brief instance data
   *
   */
  std::vector<InstanceData> data;
};

} // namespace sinen
#endif // !SINEN_DRAWABLE_HPP
