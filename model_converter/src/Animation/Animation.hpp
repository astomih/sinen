#pragma once
#include "Node.hpp"
#include <string>
#include <vector>

namespace sinen {
struct VectorKey {
  double time;
  vector3 value;
};

struct QuatKey {
  double time;
  quaternion value;
};
struct NodeAnim {
  std::string node_name;

  std::vector<VectorKey> translate;
  std::vector<VectorKey> scaling;
  std::vector<QuatKey> rotation;
};
class Animation {
public:
  float mDuration;
  std::vector<NodeAnim> body;
};
} // namespace sinen