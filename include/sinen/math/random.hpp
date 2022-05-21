#pragma once
#include "math.hpp"
#include "vector2.hpp"
#include "vector3.hpp"

namespace nen {
class random {
public:
  // Seed the generator with the specified int
  // NOTE: You should generally not need to manually use this
  static void seed(unsigned int seed);

  // Get a float between 0.0f and 1.0f
  static float get_float();

  // Get a float from the specified range
  static float get_float_range(float min, float max);

  // Get an int from the specified range
  static int get_int_range(int min, int max);

  // Get a random vector given the min/max bounds
  static vector2 get_vector(const vector2 &min, const vector2 &max);
  static vector3 get_vector(const vector3 &min, const vector3 &max);
};
} // namespace nen
