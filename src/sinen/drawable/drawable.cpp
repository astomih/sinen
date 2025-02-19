// internal
#include <drawable/drawable.hpp>

namespace sinen {
Drawable::Drawable() {
  float f[16] = {};
  this->param.user = matrix4(f);
}
Drawable::~Drawable() {}
void Drawable::world_to_instance_data(const matrix4 &mat, InstanceData &data) {

  int l = 0;
  for (int i = 0; i < 4; i++) {
    data.world_matrix_1[i] = mat.mat[l][i];
  }
  l++;
  for (int i = 0; i < 4; i++) {
    data.world_matrix_2[i] = mat.mat[l][i];
  }
  l++;
  for (int i = 0; i < 4; i++) {
    data.world_matrix_3[i] = mat.mat[l][i];
  }
  l++;
  for (int i = 0; i < 4; i++) {
    data.world_matrix_4[i] = mat.mat[l][i];
  }
}
} // namespace sinen
