#ifndef UNIFORM_DATA_HPP
#define UNIFORM_DATA_HPP

#include <glm/mat4x4.hpp>
#include <vector>
namespace sinen {
struct UniformData {
  UniformData() : data() {}
  void Add(float value) { data.push_back(value); }
  void AddMatrix(const glm::mat4 &matrix) {
    data.resize(data.size() + 16);
    memcpy(&data[data.size() - 16], &matrix, sizeof(glm::mat4));
  }
  void AddMatrices(const std::vector<glm::mat4> &matrices) {
    for (auto &m : matrices) {
      AddMatrix(m);
    }
  }
  void AddVector3(const glm::vec3 &vector) {
    data.push_back(vector.x);
    data.push_back(vector.y);
    data.push_back(vector.z);
  }
  void Change(float value, int index) { data[index] = value; }
  void Clear() { data.clear(); }
  std::vector<float> data;
};
} // namespace sinen

#endif // !UNIFORM_DATA_HPP