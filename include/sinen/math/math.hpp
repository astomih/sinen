#pragma once
#include "../time/time.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>
#include <limits>
#include <vector>

//#include <numbers>

namespace nen {
class Math {
public:
  // static constexpr float Pi = std::numbers::pi_v<float>;
  static constexpr float Pi = 3.141592f;
  static constexpr float TwoPi = Pi * 2.f;
  static constexpr float PiOver2 = Pi / 2.f;
  static constexpr float Infinity = std::numeric_limits<float>::infinity();
  static constexpr float NegInfinity = -std::numeric_limits<float>::infinity();

  static inline float ToRadians(float degrees) { return degrees * Pi / 180.0f; }

  static inline float ToDegrees(float radians) { return radians * 180.0f / Pi; }

  static inline bool NearZero(float val, float epsilon = 0.001f) {
    if (fabs(val) <= epsilon) {
      return true;
    } else {
      return false;
    }
  }

  template <typename T> static T Max(const T &a, const T &b) {
    return (a < b ? b : a);
  }

  template <typename T> static T Min(const T &a, const T &b) {
    return (a < b ? a : b);
  }

  template <typename T>
  static T Clamp(const T &value, const T &min, const T &max) {
    return std::clamp(value, min, max);
  }

  static inline float Abs(float value) { return fabs(value); }

  static inline float Cos(float angle) { return cosf(angle); }

  static inline float Sin(float angle) { return sinf(angle); }

  static inline float Tan(float angle) { return tanf(angle); }

  static inline float Acos(float value) { return acosf(value); }

  static inline float Atan2(float y, float x) { return atan2f(y, x); }

  static inline float Cot(float angle) { return 1.0f / Tan(angle); }

  static inline float Lerp(float a, float b, float f) {
    return a + f * (b - a);
  }

  static inline float Sqrt(float value) { return sqrtf(value); }

  static inline float Fmod(float numer, float denom) {
    return fmod(numer, denom);
  }

  static float Sin0_1(const float periodSec,
                      const float t = time::GetTicksAsSeconds()) {
    const auto f = Fmod(t, periodSec);
    const auto x = f / (periodSec * (1.f / (2.f * Pi)));
    return Sin(x) * 0.5f + 0.5f;
  }

  static float Cos0_1(const float periodSec,
                      const float t = time::GetTicksAsSeconds()) {
    const auto f = Fmod(t, periodSec);
    const auto x = f / (periodSec * (1.f / (2.f * Pi)));
    return Cos(x) * 0.5f + 0.5f;
  }
};
} // namespace nen
