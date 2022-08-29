#pragma once
#include "Bone.hpp"
#include <memory>
#include <sinen.hpp>
#include <string_view>

namespace sinen {
class Mesh {
public:
  vertex_array original;
  vertex_array body;

  bool has_bone;
  std::vector<Bone> bones;
};
} // namespace sinen