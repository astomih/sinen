#pragma once
#include "../vertex/vertex.hpp"
#include <vector>

namespace nen {
class vertex_array {
public:
  void push_indices(uint32_t indiceArray[], size_t count);
  std::vector<vertex> vertices;
  std::vector<uint32_t> indices;
  std::string materialName;
  uint32_t indexCount;
};
} // namespace nen