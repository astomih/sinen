#include "rect.hpp"
namespace sinen {
Ptr<Mesh> Rect::createMesh() {
  auto mesh = makePtr<Mesh>();
  mesh->vertices.push_back({glm::vec3(-1.000000, -1.000000, 0.000000),
                            glm::vec3(0.000000, 0.000000, 1.000000),
                            glm::vec2(0.000000, 1.000000),
                            Color{1.000000, 1.000000, 1.000000, 1.000000}});
  mesh->vertices.push_back({glm::vec3(1.000000, -1.000000, 0.000000),
                            glm::vec3(0.000000, 0.000000, 1.000000),
                            glm::vec2(1.000000, 1.000000),
                            Color{1.000000, 1.000000, 1.000000, 1.000000}});
  mesh->vertices.push_back({glm::vec3(1.000000, 1.000000, 0.000000),
                            glm::vec3(0.000000, 0.000000, 1.000000),
                            glm::vec2(1.000000, 0.000000),
                            Color{1.000000, 1.000000, 1.000000, 1.000000}});
  mesh->vertices.push_back({glm::vec3(-1.000000, 1.000000, 0.000000),
                            glm::vec3(0.000000, 0.000000, 1.000000),
                            glm::vec2(0.000000, 0.000000),
                            Color{1.000000, 1.000000, 1.000000, 1.000000}});
  uint32_t indices[] = {
      0, 1, 2, 0, 2, 3,
  };
  for (size_t i = 0; i < sizeof(indices) / sizeof(uint32); i++) {
    mesh->indices.push_back(indices[i]);
  }
  return mesh;
}

} // namespace sinen