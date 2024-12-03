// internal
#include <color/hsv.hpp>
namespace sinen {
void Hsv::from_color(const Color &c) {

  float r = c.r;
  float g = c.g;
  float b = c.b;
  float max = std::max(r, std::max(g, b));
  float min = std::min(r, std::min(g, b));
  float delta = max - min;
  if (delta == 0) {
    h = 0;
  } else if (max == r) {
    h = 60 * (g - b) / delta;
  } else if (max == g) {
    h = 60 * (b - r) / delta + 120;
  } else if (max == b) {
    h = 60 * (r - g) / delta + 240;
  }
  if (h < 0) {
    h += 360;
  }
  if (max == 0) {
    s = 0;
  } else {
    s = delta / max;
  }
  v = max;
}

Color Hsv::to_color() const {
  float r, g, b;
  float c = v * s;
  float h_ = h / 60;
  float x = c * (1 - std::abs(std::fmod(h_, 2) - 1));
  if (h_ < 1) {
    r = c;
    g = x;
    b = 0;
  } else if (h_ < 2) {
    r = x;
    g = c;
    b = 0;
  } else if (h_ < 3) {
    r = 0;
    g = c;
    b = x;
  } else if (h_ < 4) {
    r = 0;
    g = x;
    b = c;
  } else if (h_ < 5) {
    r = x;
    g = 0;
    b = c;
  } else {
    r = c;
    g = 0;
    b = x;
  }
  float m = v - c;
  return Color((r + m), (g + m), (b + m), 1);
}
} // namespace sinen
