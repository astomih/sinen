#include <Math/Random.hpp>
namespace nen {
void random::Init() {
  std::random_device rd;
  random::Seed(rd());
}

void random::Seed(unsigned int seed) { sGenerator.seed(seed); }

float random::GetFloat() { return GetFloatRange(0.0f, 1.0f); }

float random::GetFloatRange(float min, float max) {
  std::uniform_real_distribution<float> dist(min, max);
  return dist(sGenerator);
}

int random::GetIntRange(int min, int max) {
  std::uniform_int_distribution<int> dist(min, max);
  return dist(sGenerator);
}

vector2 random::GetVector(const vector2 &min, const vector2 &max) {
  vector2 r = vector2(GetFloat(), GetFloat());
  return min + (max - min) * r;
}

vector3 random::GetVector(const vector3 &min, const vector3 &max) {
  vector3 r = vector3(GetFloat(), GetFloat(), GetFloat());
  return min + (max - min) * r;
}

} // namespace nen