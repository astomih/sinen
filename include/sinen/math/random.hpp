#pragma once
#include "math.hpp"
#include "vector2.hpp"
#include "vector3.hpp"
#include <random>

namespace nen {
class random {
public:
  void Init();

  // Seed the generator with the specified int
  // NOTE: You should generally not need to manually use this
  void Seed(unsigned int seed);

  // Get a float between 0.0f and 1.0f
  float GetFloat();

  // Get a float from the specified range
  float GetFloatRange(float min, float max);

  // Get an int from the specified range
  int GetIntRange(int min, int max);

  // Get a random vector given the min/max bounds
  vector2 GetVector(const vector2 &min, const vector2 &max);
  vector3 GetVector(const vector3 &min, const vector3 &max);

private:
  std::mt19937 sGenerator;
};
} // namespace nen
