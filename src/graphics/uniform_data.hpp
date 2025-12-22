#ifndef UNIFORM_DATA_HPP
#define UNIFORM_DATA_HPP

#include "camera/camera.hpp"
#include <core/data/array.hpp>

#include <glm/mat4x4.hpp>
namespace sinen {
struct UniformData {
  UniformData() : data() {}
  void add(float value) { data.push_back(value); }
  void addMatrix(const glm::mat4 &matrix) {
    data.resize(data.size() + 16);
    memcpy(&data[data.size() - 16], &matrix, sizeof(glm::mat4));
  }
  void addMatrices(const Array<glm::mat4> &matrices) {
    for (auto &m : matrices) {
      addMatrix(m);
    }
  }
  void addVec3(const glm::vec3 &vector) {
    data.push_back(vector.x);
    data.push_back(vector.y);
    data.push_back(vector.z);
  }

  void addCamera(const Camera &camera) {
    addMatrix(glm::transpose(camera.getView()));
    addMatrix(glm::transpose(camera.getProjection()));
  }
  void change(float value, int index) { data[index] = value; }
  void clear() { data.clear(); }
  Array<float> data;
};
} // namespace sinen

#endif // !UNIFORM_DATA_HPP