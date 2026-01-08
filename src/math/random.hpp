#ifndef SINEN_RANDOM_HPP
#define SINEN_RANDOM_HPP
namespace sinen {
class Vec2;
class Vec3;
class Random {
public:
  // Seed the generator with the specified int
  // NOTE: You should generally not need to manually use this
  static void seed(unsigned int seed);

  // Get a float between 0.0f and 1.0f
  static float getFloat();

  // Get a float from the specified range
  static float getRange(float min, float max);

  // Get an int from the specified range
  static int getIntRange(int min, int max);

  // Get a random vector given the min/max bounds
  static Vec2 getVector(const Vec2 &min, const Vec2 &max);
  static Vec3 getVector(const Vec3 &min, const Vec3 &max);
};
} // namespace sinen
#endif // !SINEN_RANDOM_HPP
