#ifndef SINEN_BONE_HPP
#define SINEN_BONE_HPP
#include "weight.hpp"
#include <sinen/math/math.hpp>
#include <sinen/math/matrix4.hpp>
#include <sinen/math/vector2.hpp>
#include <sinen/math/vector3.hpp>

namespace sinen {
struct Bone {
  std::string name;
  matrix4 offset;

  std::vector<Weight> weights;
};
} // namespace sinen
#endif // SINEN_BONE_HPP