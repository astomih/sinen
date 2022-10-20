#ifndef SINEN_BONE_HPP
#define SINEN_BONE_HPP
#include "weight.hpp"
#include <sinen/sinen.hpp>
namespace sinen {
struct Bone {
  std::string name;
  matrix4 offset;

  std::vector<Weight> weights;
};
} // namespace sinen
#endif // SINEN_BONE_HPP