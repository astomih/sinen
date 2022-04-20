#pragma once
#include "Bone.hpp"
#include "Vertex/Vertex.hpp"
#include "Vertex/VertexArray.hpp"
#include <Nen.hpp>
#include <memory>
#include <string_view>

namespace nen {
class Mesh {
public:
  vertex_array original;
  vertex_array body;

  bool has_bone;
  std::vector<Bone> bones;
};
} // namespace nen