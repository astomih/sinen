#include "default_model_creator.h"
using namespace sinen;
vertex_array create_sprite_vertices() {
  vertex_array v_array;
  v_array.vertices.push_back({vector3(-1.000000, -1.000000, 0.000000),
                              vector3(0.000000, 0.000000, 1.000000),
                              vector2(0.000000, 1.000000),
                              color{1.000000, 1.000000, 1.000000, 1.000000}});
  v_array.vertices.push_back({vector3(1.000000, -1.000000, 0.000000),
                              vector3(0.000000, 0.000000, 1.000000),
                              vector2(1.000000, 1.000000),
                              color{1.000000, 1.000000, 1.000000, 1.000000}});
  v_array.vertices.push_back({vector3(1.000000, 1.000000, 0.000000),
                              vector3(0.000000, 0.000000, 1.000000),
                              vector2(1.000000, 0.000000),
                              color{1.000000, 1.000000, 1.000000, 1.000000}});
  v_array.vertices.push_back({vector3(-1.000000, 1.000000, 0.000000),
                              vector3(0.000000, 0.000000, 1.000000),
                              vector2(0.000000, 0.000000),
                              color{1.000000, 1.000000, 1.000000, 1.000000}});
  uint32_t indices[] = {
      0, 1, 2, 0, 2, 3,
  };
  v_array.indexCount = sizeof(indices) / sizeof(uint32_t);
  v_array.push_indices(indices, v_array.indexCount);
  return v_array;
}
