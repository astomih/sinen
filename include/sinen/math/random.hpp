#ifndef SINEN_RANDOM_HPP
#define SINEN_RANDOM_HPP
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace sinen {
class Random {
public:
  // Seed the generator with the specified int
  // NOTE: You should generally not need to manually use this
  static void seed(unsigned int seed);

  // Get a float between 0.0f and 1.0f
  static float get_float();

  // Get a float from the specified range
  static float GetRange(float min, float max);

  // Get an int from the specified range
  static int GetIntRange(int min, int max);

  // Get a random vector given the min/max bounds
  static glm::vec2 get_vector(const glm::vec2 &min, const glm::vec2 &max);
  static glm::vec3 get_vector(const glm::vec3 &min, const glm::vec3 &max);
};
} // namespace sinen
#endif // !SINEN_RANDOM_HPP
