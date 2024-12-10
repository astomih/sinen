#ifndef SINEN_INSTANCE_DATA_HPP
#define SINEN_INSTANCE_DATA_HPP
#include "../color/color.hpp"
namespace sinen {
/**
 * @brief Instance data for Instancing draw
 *
 */
struct InstanceData {
  float world_matrix_1[4];
  float world_matrix_2[4];
  float world_matrix_3[4];
  float world_matrix_4[4];
};

} // namespace sinen
#endif // !SINEN_INSTANCE_DATA_HPP
