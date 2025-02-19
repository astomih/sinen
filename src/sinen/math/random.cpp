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

Vector2 Random::get_vector(const Vector2 &min, const Vector2 &max) {
  Vector2 r = Vector2(get_float(), get_float());
  return min + (max - min) * r;
}

Vector3 Random::get_vector(const Vector3 &min, const Vector3 &max) {
  Vector3 r = Vector3(get_float(), get_float(), get_float());
  return min + (max - min) * r;
}

} // namespace sinen
