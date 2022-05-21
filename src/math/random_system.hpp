#pragma once
#include <random>

namespace nen {
class random_system {
public:
  void init();

  std::mt19937 sGenerator;
};
} // namespace nen