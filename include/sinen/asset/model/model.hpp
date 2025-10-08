#ifndef SINEN_MODEL_HPP
#define SINEN_MODEL_HPP
#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

#include "../../physics/collision.hpp"
#include "graphics/uniform_data.hpp"
#include "mesh.hpp"
#include <asset/texture/material.hpp>

namespace sinen {
struct Model {
public:
  /**
   * @brief Construct a new model object
   *
   */
  Model();
  /**
   * @brief Destroy the model object
   *
   */
  ~Model() = default;
  /**
   * @brief Load the model from a file
   *
   * @brief Model format is a custom format(.sim)
   * @param str
   */
  void load(std::string_view str);
  void loadFromVertexArray(const Mesh &vArray);
  void loadSprite();
  void loadBox();
  void play(float start);
  void update(float delta_time);
  AABB &getAABB() const;
  std::shared_ptr<void> data;
  std::vector<Vertex> allVertex() const;
  std::vector<std::uint32_t> allIndices() const;
  UniformData getBoneUniformData() const;

  Material getMaterial() const { return material; }

private:
  void loadBoneUniform(float time);
  float time = 0.0f;
  std::vector<glm::mat4> inverseBindMatrices;
  Material material;
};
} // namespace sinen
#endif // !SINEN_MODEL_HPP
