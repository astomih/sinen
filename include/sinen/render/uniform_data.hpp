#ifndef UNIFORM_DATA_HPP
#define UNIFORM_DATA_HPP
#include <math/matrix4.hpp>
#include <math/vector3.hpp>
#include <vector>
namespace sinen {
struct UniformData {
  UniformData() : data() {}
  void add(float value) { data.push_back(value); }
  void add_matrix(const matrix4 &matrix) {
    data.insert(data.end(), matrix.mat.m16, matrix.mat.m16 + 16);
  }
  void add_matrices(const std::vector<matrix4> &matrices) {
    for (auto &matrix : matrices) {
      data.insert(data.end(), matrix.mat.m16, matrix.mat.m16 + 16);
    }
  }
  void add_vector3(const Vector3 &vector) {
    data.push_back(vector.x);
    data.push_back(vector.y);
    data.push_back(vector.z);
  }
  void change(float value, int index) { data[index] = value; }
  std::vector<float> data;
};
} // namespace sinen

#endif // !UNIFORM_DATA_HPP