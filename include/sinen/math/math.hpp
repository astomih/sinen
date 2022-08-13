#ifndef SINEN_MATH_HPP
#define SINEN_MATH_HPP

#include "../time/time.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>
#include <limits>
#include <vector>

// C++20
//#include <numbers>

namespace sinen {
class math {
public:
  // static constexpr float Pi = std::numbers::pi_v<float>;
  static constexpr float pi = 3.141592f;
  static constexpr float two_pi = pi * 2.f;
  static constexpr float pi_over2 = pi / 2.f;
  static constexpr float infinity = std::numeric_limits<float>::infinity();
  static constexpr float neg_infinity = -std::numeric_limits<float>::infinity();

  static inline float to_radians(float degrees) {
    return degrees * pi / 180.0f;
  }

  static inline float to_degrees(float radians) {
    return radians * 180.0f / pi;
  }

  static inline bool near_zero(float val, float epsilon = 0.001f) {
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

  static inline float abs(float value) { return fabs(value); }

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
    return fmod(numer, denom);
  }

  static float sin_0_1(const float periodSec,
                       const float t = time::get_ticks_as_seconds()) {
    const auto f = fmod(t, periodSec);
    const auto x = f / (periodSec * (1.f / (2.f * pi)));
    return sin(x) * 0.5f + 0.5f;
  }

  static float cos_0_1(const float periodSec,
                       const float t = time::get_ticks_as_seconds()) {
    const auto f = fmod(t, periodSec);
    const auto x = f / (periodSec * (1.f / (2.f * pi)));
    return cos(x) * 0.5f + 0.5f;
  }
};
} // namespace sinen
#endif // !SINEN_MATH_HPP