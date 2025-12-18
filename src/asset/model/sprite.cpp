#include "default_model_creator.hpp"
namespace sinen {
Mesh createSpriteVertices() {
  Mesh v_array;
  v_array.vertices.push_back({glm::vec3(-1.000000, -1.000000, 0.000000),
                              glm::vec3(0.000000, 0.000000, 1.000000),
                              glm::vec2(0.000000, 1.000000),
                              Color{1.000000, 1.000000, 1.000000, 1.000000}});
  v_array.vertices.push_back({glm::vec3(1.000000, -1.000000, 0.000000),
                              glm::vec3(0.000000, 0.000000, 1.000000),
                              glm::vec2(1.000000, 1.000000),
                              Color{1.000000, 1.000000, 1.000000, 1.000000}});
  v_array.vertices.push_back({glm::vec3(1.000000, 1.000000, 0.000000),
                              glm::vec3(0.000000, 0.000000, 1.000000),
                              glm::vec2(1.000000, 0.000000),
                              Color{1.000000, 1.000000, 1.000000, 1.000000}});
  v_array.vertices.push_back({glm::vec3(-1.000000, 1.000000, 0.000000),
                              glm::vec3(0.000000, 0.000000, 1.000000),
                              glm::vec2(0.000000, 0.000000),
                              Color{1.000000, 1.000000, 1.000000, 1.000000}});
  uint32_t indices[] = {
      0, 1, 2, 0, 2, 3,
  };
  v_array.indexCount = sizeof(indices) / sizeof(uint32_t);
  v_array.pushIndices(indices, v_array.indexCount);
  return v_array;
}

} // namespace sinen