#pragma once
#include "../Actor/Actor.hpp"
#include "../Window/Window.hpp"
#include "Math.hpp"
#include <cstdio>
#include <string>

namespace nen {
class transform {
public:
  explicit transform(class base_actor &owner) : mOwner(owner) {}
  void LookAt(const vector3 &target, const vector3 &norm);

private:
  base_actor &mOwner;
};
} // namespace nen
