#ifndef SINEN_MODEL_HPP
#define SINEN_MODEL_HPP
#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

#include "../physics/collision.hpp"
#include "math/matrix4.hpp"
#include "physics/primitive3.hpp"
#include "render/uniform_data.hpp"
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
  void load(std::string_view str);
  void load_from_vertex_array(const VertexArray &vArray);
  void load_sprite();
  void load_box();
  void play(float start);
  void update(float delta_time);
  AABB &aabb() const;
  std::shared_ptr<void> data;
  std::vector<Vertex> all_vertex() const;
  std::vector<std::uint32_t> all_indices() const;
  UniformData bone_uniform_data() const;

private:
  void load_bone_uniform(float time);
  float time = 0.0f;
  std::vector<matrix4> inverse_bind_matrices;
};
} // namespace sinen
#endif // !SINEN_MODEL_HPP
