#pragma  once
#include <random>
#include "Math.hpp"

class Random
{
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
	static Vector2f GetVector(const Vector2f& min, const Vector2f& max);
	static Vector3f GetVector(const Vector3f& min, const Vector3f& max);
private:
	static std::mt19937 sGenerator;
};
