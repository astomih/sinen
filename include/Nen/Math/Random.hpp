#pragma once
#include "Math.hpp"
#include "Vector2.hpp"
#include "Vector3.hpp"
#include <random>

namespace nen {
class random {
public:
  static void Init();

  // Seed the generator with the specified int
  // NOTE: You should generally not need to manually use this
  static void Seed(unsigned int seed);

  // Get a float between 0.0f and 1.0f
  static float GetFloat();

  // Get a float from the specified range
  static float GetFloatRange(float min, float max);

  // Get an int from the specified range
  static int GetIntRange(int min, int max);

  // Get a random vector given the min/max bounds
  static vector2 GetVector(const vector2 &min, const vector2 &max);
  static vector3 GetVector(const vector3 &min, const vector3 &max);

private:
  static std::mt19937 sGenerator;
};
} // namespace nen
