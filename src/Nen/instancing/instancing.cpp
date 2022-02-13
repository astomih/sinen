#include "instancing/instancing.hpp"
#include <Nen.hpp>

namespace nen {
void instancing::world_to_instance_data(const matrix4 &mat,
                                        instance_data &data) {

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
} // namespace nen
