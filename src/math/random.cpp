#include "../manager/get_system.hpp"
#include "random_system.hpp"
#include <math/random.hpp>
#include <random>
namespace nen {
std::mt19937 sGenerator;
void random_system::init() {
  std::random_device rd;
  random::seed(rd());
}

void random::seed(unsigned int seed) { sGenerator.seed(seed); }

float random::get_float() { return get_float_range(0.0f, 1.0f); }

float random::get_float_range(float min, float max) {
  std::uniform_real_distribution<float> dist(min, max);
  return dist(get_random().sGenerator);
}

int random::get_int_range(int min, int max) {
  std::uniform_int_distribution<int> dist(min, max);
  int a = dist(sGenerator);
  return a;
}

vector2 random::get_vector(const vector2 &min, const vector2 &max) {
  vector2 r = vector2(get_float(), get_float());
  return min + (max - min) * r;
}

vector3 random::get_vector(const vector3 &min, const vector3 &max) {
  vector3 r = vector3(get_float(), get_float(), get_float());
  return min + (max - min) * r;
}

} // namespace nen