#ifndef SINEN_DRAWABLE_HPP
#define SINEN_DRAWABLE_HPP
#include <glm/mat4x4.hpp>

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
    glm::mat4 world;
    glm::mat4 view;
    glm::mat4 proj;
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
  void world_to_instance_data(const glm::mat4 &mat, InstanceData &data);
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
