#pragma once
#include "Weight.hpp"
#include <sinen.hpp>

namespace sinen {
struct Bone {
  std::string name;
  matrix4 offset;

  std::vector<Weight> weights;
};
} // namespace sinen
