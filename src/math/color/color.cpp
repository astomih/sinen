// internal
#include <math/color/color.hpp>
#include <math/color/palette.hpp>

namespace sinen {
const Color Palette::black() { return Color(0.f); }
const Color Palette::light_black() { return Color(0.2f); }
const Color Palette::white() { return Color(1.f); }
const Color Palette::red() { return Color(1.f, 0.f, 0.f, 1.f); }
const Color Palette::green() { return Color(0.f, 1.f, 0.f, 1.f); }
const Color Palette::blue() { return Color(0.f, 0.f, 1.f, 1.f); }
const Color Palette::yellow() { return Color(1.f, 1.f, 0.f, 1.f); }
const Color Palette::light_yellow() { return Color(1.f, 1.f, 0.88f, 1.f); }
const Color Palette::light_blue() { return Color(0.68f, 0.85f, 0.9f, 1.f); }
const Color Palette::light_pink() { return Color(1.f, 0.71f, 0.76f, 1.f); }
const Color Palette::light_green() { return Color(0.56f, 0.93f, 0.56f, 1.f); }
} // namespace sinen
