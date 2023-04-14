// internal
#include <color/color.hpp>
#include <color/palette.hpp>

namespace sinen {
const color palette::black() { return color(0.f); }
const color palette::light_black() { return color(0.2f); }
const color palette::white() { return color(1.f); }
const color palette::red() { return color(1.f, 0.f, 0.f, 1.f); }
const color palette::green() { return color(0.f, 1.f, 0.f, 1.f); }
const color palette::blue() { return color(0.f, 0.f, 1.f, 1.f); }
const color palette::yellow() { return color(1.f, 1.f, 0.f, 1.f); }
const color palette::light_yellow() { return color(1.f, 1.f, 0.88f, 1.f); }
const color palette::light_blue() { return color(0.68f, 0.85f, 0.9f, 1.f); }
const color palette::light_pink() { return color(1.f, 0.71f, 0.76f, 1.f); }
const color palette::light_green() { return color(0.56f, 0.93f, 0.56f, 1.f); }
} // namespace sinen
