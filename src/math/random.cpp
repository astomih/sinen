#include "random.hpp"
#include <math/random.hpp>
#include <math/vector.hpp>
#include <random>
namespace sinen {
std::mt19937 Random::sGenerator;
bool Random::initialize() {
  std::random_device rd;
  Random::seed(rd());
  return true;
}
void Random::shutdown() {}

void Random::seed(unsigned int seed) { sGenerator.seed(seed); }

float Random::getFloat() { return getRange(0.0f, 1.0f); }

float Random::getRange(float min, float max) {
  std::uniform_real_distribution<float> dist(min, max);
  return dist(sGenerator);
}

int Random::getIntRange(int min, int max) {
  std::uniform_int_distribution<int> dist(min, max);
  int a = dist(sGenerator);
  return a;
}

Vec2 Random::getVector(const Vec2 &min, const Vec2 &max) {
  Vec2 r = Vec2(getFloat(), getFloat());
  return min + (max - min) * r;
}

Vec3 Random::getVector(const Vec3 &min, const Vec3 &max) {
  Vec3 r = Vec3(getFloat(), getFloat(), getFloat());
  return min + (max - min) * r;
}

} // namespace sinen
