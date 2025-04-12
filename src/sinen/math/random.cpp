#include "random_system.hpp"
#include <math/random.hpp>
#include <random>
namespace sinen {
std::mt19937 random_system::sGenerator;
bool random_system::initialize() {
  std::random_device rd;
  Random::seed(rd());
  return true;
}
void random_system::shutdown() {}

void Random::seed(unsigned int seed) { random_system::sGenerator.seed(seed); }

float Random::get_float() { return get_float_range(0.0f, 1.0f); }

float Random::get_float_range(float min, float max) {
  std::uniform_real_distribution<float> dist(min, max);
  return dist(random_system::sGenerator);
}

int Random::get_int_range(int min, int max) {
  std::uniform_int_distribution<int> dist(min, max);
  int a = dist(random_system::sGenerator);
  return a;
}

glm::vec2 Random::get_vector(const glm::vec2 &min, const glm::vec2 &max) {
  glm::vec2 r = glm::vec2(get_float(), get_float());
  return min + (max - min) * r;
}

glm::vec3 Random::get_vector(const glm::vec3 &min, const glm::vec3 &max) {
  glm::vec3 r = glm::vec3(get_float(), get_float(), get_float());
  return min + (max - min) * r;
}

} // namespace sinen
