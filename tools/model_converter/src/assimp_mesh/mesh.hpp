#ifndef SINEN_MESH_HPP
#define SINEN_MESH_HPP

#include "bone.hpp"
#include <memory>
#include <sinen/sinen.hpp>
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
#endif // SINEN_MESH_HPP