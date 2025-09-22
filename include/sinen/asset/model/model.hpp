#ifndef SINEN_MODEL_HPP
#define SINEN_MODEL_HPP
#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

#include "../../physics/collision.hpp"
#include "graphics/uniform_data.hpp"
#include "vertex_array.hpp"

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
  void Load(std::string_view str) const;
  void LoadFromVertexArray(const VertexArray &vArray);
  void LoadSprite();
  void LoadBox();
  void Play(float start);
  void Update(float delta_time);
  AABB &GetAABB() const;
  std::shared_ptr<void> data;
  std::vector<Vertex> AllVertex() const;
  std::vector<std::uint32_t> AllIndices() const;
  UniformData GetBoneUniformData() const;

private:
  void LoadBoneUniform(float time);
  float time = 0.0f;
  std::vector<glm::mat4> inverseBindMatrices;
};
} // namespace sinen
#endif // !SINEN_MODEL_HPP
