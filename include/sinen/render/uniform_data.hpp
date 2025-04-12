#ifndef UNIFORM_DATA_HPP
#define UNIFORM_DATA_HPP
#include <math/matrix4.hpp>
#include <math/vector3.hpp>

#include <glm/mat4x4.hpp>
#include <vector>
namespace sinen {
struct UniformData {
  UniformData() : data() {}
  void add(float value) { data.push_back(value); }
  void add_matrix(const glm::mat4 &matrix) {
    data.resize(data.size() + 16);
    memcpy(&data[data.size() - 16], &matrix, sizeof(glm::mat4));
  }
  void add_matrices(const std::vector<glm::mat4> &matrices) {
    for (auto &m : matrices) {
      add_matrix(m);
    }
  }
  void add_vector3(const glm::vec3 &vector) {
    data.push_back(vector.x);
    data.push_back(vector.y);
    data.push_back(vector.z);
  }
  void change(float value, int index) { data[index] = value; }
  void clear() { data.clear(); }
  std::vector<float> data;
};
} // namespace sinen

#endif // !UNIFORM_DATA_HPP