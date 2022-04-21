#pragma once
#include "../math/vector3.hpp"
namespace nen {
class color {
public:
  constexpr color(const float red, const float green, const float blue,
                  const float alpha)
      : r(red), g(green), b(blue), a(alpha) {}
  constexpr color(const float value) : r(value), g(value), b(value), a(1.f) {}
  color() = default;

  float r;
  float g;
  float b;
  float a;
};
class palette {
public:
  static const color Black;
  static const color LightBlack;
  static const color White;
  static const color Red;
  static const color Green;
  static const color Blue;
  static const color Yellow;
  static const color LightYellow;
  static const color LightBlue;
  static const color LightPink;
  static const color LightGreen;
};

} // namespace nen