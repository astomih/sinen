#include "default_model_creator.hpp"
using namespace sinen;
VertexArray create_sprite_vertices() {
  VertexArray v_array;
  v_array.vertices.push_back({Vector3(-1.000000, -1.000000, 0.000000),
                              Vector3(0.000000, 0.000000, 1.000000),
                              Vector2(0.000000, 1.000000),
                              Color{1.000000, 1.000000, 1.000000, 1.000000}});
  v_array.vertices.push_back({Vector3(1.000000, -1.000000, 0.000000),
                              Vector3(0.000000, 0.000000, 1.000000),
                              Vector2(1.000000, 1.000000),
                              Color{1.000000, 1.000000, 1.000000, 1.000000}});
  v_array.vertices.push_back({Vector3(1.000000, 1.000000, 0.000000),
                              Vector3(0.000000, 0.000000, 1.000000),
                              Vector2(1.000000, 0.000000),
                              Color{1.000000, 1.000000, 1.000000, 1.000000}});
  v_array.vertices.push_back({Vector3(-1.000000, 1.000000, 0.000000),
                              Vector3(0.000000, 0.000000, 1.000000),
                              Vector2(0.000000, 0.000000),
                              Color{1.000000, 1.000000, 1.000000, 1.000000}});
  uint32_t indices[] = {
      0, 1, 2, 0, 2, 3,
  };
  v_array.indexCount = sizeof(indices) / sizeof(uint32_t);
  v_array.push_indices(indices, v_array.indexCount);
  return v_array;
}
