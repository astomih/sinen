#include "default_model_creator.h"
nen::vertex_array create_box_vertices() {
  nen::vertex_array v_array;
  v_array.vertices.push_back(
      {nen::vector3(1.000000, 1.000000, 1.000000),
       nen::vector3(0.000000, 0.000000, 1.000000),
       nen::vector2(0.625000, 0.500000),
       nen::color{1.000000, 1.000000, 1.000000, 1.000000}});
  v_array.vertices.push_back(
      {nen::vector3(-1.000000, 1.000000, 1.000000),
       nen::vector3(0.000000, 0.000000, 1.000000),
       nen::vector2(0.875000, 0.500000),
       nen::color{1.000000, 1.000000, 1.000000, 1.000000}});
  v_array.vertices.push_back(
      {nen::vector3(-1.000000, -1.000000, 1.000000),
       nen::vector3(0.000000, 0.000000, 1.000000),
       nen::vector2(0.875000, 0.250000),
       nen::color{1.000000, 1.000000, 1.000000, 1.000000}});
  v_array.vertices.push_back(
      {nen::vector3(1.000000, -1.000000, 1.000000),
       nen::vector3(0.000000, 0.000000, 1.000000),
       nen::vector2(0.625000, 0.250000),
       nen::color{1.000000, 1.000000, 1.000000, 1.000000}});
  v_array.vertices.push_back(
      {nen::vector3(1.000000, -1.000000, -1.000000),
       nen::vector3(0.000000, -1.000000, 0.000000),
       nen::vector2(0.375000, 0.250000),
       nen::color{1.000000, 1.000000, 1.000000, 1.000000}});
  v_array.vertices.push_back(
      {nen::vector3(1.000000, -1.000000, 1.000000),
       nen::vector3(0.000000, -1.000000, 0.000000),
       nen::vector2(0.625000, 0.250000),
       nen::color{1.000000, 1.000000, 1.000000, 1.000000}});
  v_array.vertices.push_back(
      {nen::vector3(-1.000000, -1.000000, 1.000000),
       nen::vector3(0.000000, -1.000000, 0.000000),
       nen::vector2(0.625000, 0.000000),
       nen::color{1.000000, 1.000000, 1.000000, 1.000000}});
  v_array.vertices.push_back(
      {nen::vector3(-1.000000, -1.000000, -1.000000),
       nen::vector3(0.000000, -1.000000, 0.000000),
       nen::vector2(0.375000, 0.000000),
       nen::color{1.000000, 1.000000, 1.000000, 1.000000}});
  v_array.vertices.push_back(
      {nen::vector3(-1.000000, -1.000000, -1.000000),
       nen::vector3(-1.000000, 0.000000, 0.000000),
       nen::vector2(0.375000, 1.000000),
       nen::color{1.000000, 1.000000, 1.000000, 1.000000}});
  v_array.vertices.push_back(
      {nen::vector3(-1.000000, -1.000000, 1.000000),
       nen::vector3(-1.000000, 0.000000, 0.000000),
       nen::vector2(0.625000, 1.000000),
       nen::color{1.000000, 1.000000, 1.000000, 1.000000}});
  v_array.vertices.push_back(
      {nen::vector3(-1.000000, 1.000000, 1.000000),
       nen::vector3(-1.000000, 0.000000, 0.000000),
       nen::vector2(0.625000, 0.750000),
       nen::color{1.000000, 1.000000, 1.000000, 1.000000}});
  v_array.vertices.push_back(
      {nen::vector3(-1.000000, 1.000000, -1.000000),
       nen::vector3(-1.000000, 0.000000, 0.000000),
       nen::vector2(0.375000, 0.750000),
       nen::color{1.000000, 1.000000, 1.000000, 1.000000}});
  v_array.vertices.push_back(
      {nen::vector3(-1.000000, 1.000000, -1.000000),
       nen::vector3(0.000000, 0.000000, -1.000000),
       nen::vector2(0.125000, 0.500000),
       nen::color{1.000000, 1.000000, 1.000000, 1.000000}});
  v_array.vertices.push_back(
      {nen::vector3(1.000000, 1.000000, -1.000000),
       nen::vector3(0.000000, 0.000000, -1.000000),
       nen::vector2(0.375000, 0.500000),
       nen::color{1.000000, 1.000000, 1.000000, 1.000000}});
  v_array.vertices.push_back(
      {nen::vector3(1.000000, -1.000000, -1.000000),
       nen::vector3(0.000000, 0.000000, -1.000000),
       nen::vector2(0.375000, 0.250000),
       nen::color{1.000000, 1.000000, 1.000000, 1.000000}});
  v_array.vertices.push_back(
      {nen::vector3(-1.000000, -1.000000, -1.000000),
       nen::vector3(0.000000, 0.000000, -1.000000),
       nen::vector2(0.125000, 0.250000),
       nen::color{1.000000, 1.000000, 1.000000, 1.000000}});
  v_array.vertices.push_back(
      {nen::vector3(1.000000, 1.000000, -1.000000),
       nen::vector3(1.000000, 0.000000, 0.000000),
       nen::vector2(0.375000, 0.500000),
       nen::color{1.000000, 1.000000, 1.000000, 1.000000}});
  v_array.vertices.push_back(
      {nen::vector3(1.000000, 1.000000, 1.000000),
       nen::vector3(1.000000, 0.000000, 0.000000),
       nen::vector2(0.625000, 0.500000),
       nen::color{1.000000, 1.000000, 1.000000, 1.000000}});
  v_array.vertices.push_back(
      {nen::vector3(1.000000, -1.000000, 1.000000),
       nen::vector3(1.000000, 0.000000, 0.000000),
       nen::vector2(0.625000, 0.250000),
       nen::color{1.000000, 1.000000, 1.000000, 1.000000}});
  v_array.vertices.push_back(
      {nen::vector3(1.000000, -1.000000, -1.000000),
       nen::vector3(1.000000, 0.000000, 0.000000),
       nen::vector2(0.375000, 0.250000),
       nen::color{1.000000, 1.000000, 1.000000, 1.000000}});
  v_array.vertices.push_back(
      {nen::vector3(-1.000000, 1.000000, -1.000000),
       nen::vector3(0.000000, 1.000000, 0.000000),
       nen::vector2(0.375000, 0.750000),
       nen::color{1.000000, 1.000000, 1.000000, 1.000000}});
  v_array.vertices.push_back(
      {nen::vector3(-1.000000, 1.000000, 1.000000),
       nen::vector3(0.000000, 1.000000, 0.000000),
       nen::vector2(0.625000, 0.750000),
       nen::color{1.000000, 1.000000, 1.000000, 1.000000}});
  v_array.vertices.push_back(
      {nen::vector3(1.000000, 1.000000, 1.000000),
       nen::vector3(0.000000, 1.000000, 0.000000),
       nen::vector2(0.625000, 0.500000),
       nen::color{1.000000, 1.000000, 1.000000, 1.000000}});
  v_array.vertices.push_back(
      {nen::vector3(1.000000, 1.000000, -1.000000),
       nen::vector3(0.000000, 1.000000, 0.000000),
       nen::vector2(0.375000, 0.500000),
       nen::color{1.000000, 1.000000, 1.000000, 1.000000}});
  uint32_t indices[] = {
      0,  1,  2,  0,  2,  3,  4,  5,  6,  4,  6,  7,  8,  9,  10, 8,  10, 11,
      12, 13, 14, 12, 14, 15, 16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23,
  };
  v_array.indexCount = sizeof(indices) / sizeof(uint32_t);
  v_array.push_indices(indices, v_array.indexCount);
  return v_array;
}
