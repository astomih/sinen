#include "random_system.hpp"
#include <math/random.hpp>
#include <math/vector.hpp>
#include <random>
namespace sinen {
std::mt19937 RandomSystem::sGenerator;
bool RandomSystem::initialize() {
  std::random_device rd;
  Random::seed(rd());
  return true;
}
void RandomSystem::shutdown() {}

void Random::seed(unsigned int seed) { RandomSystem::sGenerator.seed(seed); }

float Random::getFloat() { return getRange(0.0f, 1.0f); }

float Random::getRange(float min, float max) {
  std::uniform_real_distribution<float> dist(min, max);
  return dist(RandomSystem::sGenerator);
}

int Random::getIntRange(int min, int max) {
  std::uniform_int_distribution<int> dist(min, max);
  int a = dist(RandomSystem::sGenerator);
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
