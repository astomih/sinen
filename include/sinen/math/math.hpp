#ifndef SINEN_MATH_HPP
#define SINEN_MATH_HPP
#include "color/color.hpp"
#include "periodic.hpp"
#include "random.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>
#include <limits>
#include <numbers>
#include <vector>

namespace sinen {
class Math {
public:
  static constexpr float pi = std::numbers::pi_v<float>;
  // 2 * pi
  static constexpr float twoPi = pi * 2.f;
  // pi / 2
  static constexpr float piOver2 = pi / 2.f;
  static constexpr float infinity = std::numeric_limits<float>::infinity();
  static constexpr float negInfinity = -std::numeric_limits<float>::infinity();

  static inline float toRadians(float degrees) { return degrees * pi / 180.0f; }

  static inline float toDegrees(float radians) { return radians * 180.0f / pi; }

  static inline bool nearZero(float val, float epsilon = 0.001f) {
    if (fabs(val) <= epsilon) {
      return true;
    } else {
      return false;
    }
  }

  template <typename T> static T max(const T &a, const T &b) {
    return (a < b ? b : a);
  }

  template <typename T> static T min(const T &a, const T &b) {
    return (a < b ? a : b);
  }

  template <typename T>
  static T clamp(const T &value, const T &min, const T &max) {
    return std::clamp(value, min, max);
  }

  static inline float abs(float value) { return std::fabs(value); }

  static inline float cos(float angle) { return cosf(angle); }

  static inline float sin(float angle) { return sinf(angle); }

  static inline float tan(float angle) { return tanf(angle); }

  static inline float acos(float value) { return acosf(value); }

  static inline float atan2(float y, float x) { return atan2f(y, x); }

  static inline float cot(float angle) { return 1.0f / tan(angle); }

  static inline float lerp(float a, float b, float f) {
    return a + f * (b - a);
  }

  static inline float sqrt(float value) { return sqrtf(value); }

  static inline float fmod(float numer, float denom) {
    return std::fmod(numer, denom);
  }
};
} // namespace sinen
#endif // !SINEN_MATH_HPP