#pragma once
#include "../Mesh/Mesh.hpp"
#include <sinen.hpp>

namespace sinen {
class Node {
public:
  std::string name;
  std::vector<Mesh> mesh;
  matrix4 matrix;
  matrix4 matrix_orig;
  matrix4 global_matrix;
  matrix4 invert_matrix;
  std::vector<std::shared_ptr<Node>> children;
};
} // namespace sinen