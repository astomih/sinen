#include "default_model_creator.h"
nen::vertex_array create_sprite_vertices() {
  nen::vertex_array v_array;
  v_array.vertices.push_back(
      {nen::vector3(-1.000000, -1.000000, 0.000000),
       nen::vector3(0.000000, 0.000000, 1.000000),
       nen::vector2(0.000000, 1.000000),
       nen::color{1.000000, 1.000000, 1.000000, 1.000000}});
  v_array.vertices.push_back(
      {nen::vector3(1.000000, -1.000000, 0.000000),
       nen::vector3(0.000000, 0.000000, 1.000000),
       nen::vector2(1.000000, 1.000000),
       nen::color{1.000000, 1.000000, 1.000000, 1.000000}});
  v_array.vertices.push_back(
      {nen::vector3(1.000000, 1.000000, 0.000000),
       nen::vector3(0.000000, 0.000000, 1.000000),
       nen::vector2(1.000000, 0.000000),
       nen::color{1.000000, 1.000000, 1.000000, 1.000000}});
  v_array.vertices.push_back(
      {nen::vector3(-1.000000, 1.000000, 0.000000),
       nen::vector3(0.000000, 0.000000, 1.000000),
       nen::vector2(0.000000, 0.000000),
       nen::color{1.000000, 1.000000, 1.000000, 1.000000}});
  uint32_t indices[] = {
      0, 1, 2, 0, 2, 3,
  };
  v_array.indexCount = sizeof(indices) / sizeof(uint32_t);
  v_array.push_indices(indices, v_array.indexCount);
  return v_array;
}
