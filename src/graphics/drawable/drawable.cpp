// internal
#include "glm/ext/matrix_float4x4.hpp"
#include <graphics/drawable/drawable.hpp>

namespace sinen {
Drawable::Drawable() {}
Drawable::~Drawable() {}
void Drawable::world_to_instance_data(const glm::mat4 &mat,
                                      InstanceData &data) {

  int l = 0;
  for (int i = 0; i < 4; i++) {
    data.world_matrix_1[i] = mat[l][i];
  }
  l++;
  for (int i = 0; i < 4; i++) {
    data.world_matrix_2[i] = mat[l][i];
  }
  l++;
  for (int i = 0; i < 4; i++) {
    data.world_matrix_3[i] = mat[l][i];
  }
  l++;
  for (int i = 0; i < 4; i++) {
    data.world_matrix_4[i] = mat[l][i];
  }
}
} // namespace sinen
