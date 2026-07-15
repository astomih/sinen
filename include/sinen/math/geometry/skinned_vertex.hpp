#ifndef SINEN_SKINNED_VERTEX_HPP
#define SINEN_SKINNED_VERTEX_HPP
#include <math/vec4.hpp>
namespace sinen {
struct SkinnedVertex {
  Vec4 boneIDs;
  Vec4 boneWeights;
};
} // namespace sinen
#endif