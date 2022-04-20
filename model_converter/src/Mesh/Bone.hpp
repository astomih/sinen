#pragma once
#include "Weight.hpp"
#include <Nen.hpp>

namespace nen {
struct Bone {
  std::string name;
  matrix4 offset;

  std::vector<Weight> weights;
};
} // namespace nen
