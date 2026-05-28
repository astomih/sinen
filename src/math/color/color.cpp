// internal
#include <math/color/color.hpp>
#include <core/data/table_string.hpp>
#include <math/color/palette.hpp>
#include <core/data/table_string.hpp>

namespace sinen {
const Color Palette::black() { return Color(0.f); }
const Color Palette::gray() { return Color(0.2f); }
const Color Palette::white() { return Color(1.f); }
const Color Palette::red() { return Color(1.f, 0.f, 0.f, 1.f); }
const Color Palette::green() { return Color(0.f, 1.f, 0.f, 1.f); }
const Color Palette::blue() { return Color(0.f, 0.f, 1.f, 1.f); }
const Color Palette::yellow() { return Color(1.f, 1.f, 0.f, 1.f); }
const Color Palette::lightYellow() { return Color(1.f, 1.f, 0.88f, 1.f); }
const Color Palette::lightBlue() { return Color(0.68f, 0.85f, 0.9f, 1.f); }
const Color Palette::lightPink() { return Color(1.f, 0.71f, 0.76f, 1.f); }
const Color Palette::lightGreen() { return Color(0.56f, 0.93f, 0.56f, 1.f); }
} // namespace sinen

namespace sinen {
String Color::tableString() const {
  TablePair p;
  p.emplace_back("r", toStringTrim(r));
  p.emplace_back("g", toStringTrim(g));
  p.emplace_back("b", toStringTrim(b));
  p.emplace_back("a", toStringTrim(a));
  return convert("sn.Color", p, false);
}
} // namespace sinen
