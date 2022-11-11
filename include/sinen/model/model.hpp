#ifndef SINEN_MODEL_HPP
#define SINEN_MODEL_HPP
#include <cstdint>
#include <string_view>
#include <vector>

#include "../physics/collision.hpp"
#include "../vertex/vertex_array.hpp"

namespace sinen {
struct model {
public:
  /**
   * @brief Construct a new model object
   *
   */
  model() : parent(nullptr){};
  /**
   * @brief Destroy the model object
   *
   */
  ~model() = default;
  /**
   * @brief Load the model from a file
   *
   * @brief Model format is a custom format(.sim)
   * @param str
   * @param name
   */
  void load(std::string_view str, std::string_view name);
  std::vector<vertex> all_vertex() const;
  std::vector<std::uint32_t> all_indices() const;

  aabb local_aabb;
  model *parent;
  std::vector<model *> children;
  std::string name;
  vertex_array v_array;
};
} // namespace sinen
#endif // !SINEN_MODEL_HPP
