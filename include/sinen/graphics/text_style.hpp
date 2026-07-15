#ifndef SINEN_TEXT_STYLE_HPP
#define SINEN_TEXT_STYLE_HPP

#include <core/data/ptr.hpp>
#include <graphics/font/font.hpp>
#include <math/color/color.hpp>
#include <math/color/palette.hpp>
#include <math/geometry/pivot.hpp>
#include <math/vec2.hpp>

namespace sinen {
class TextStyle {
public:
  TextStyle() = default;
  TextStyle(Font &font, const Color &color = Palette::white(),
            float fontSize = 32.0f)
      : fontRef(&font), color(color), fontSize(fontSize) {}
  TextStyle(const Ptr<Font> &font, const Color &color = Palette::white(),
            float fontSize = 32.0f)
      : fontPtr(font), fontRef(font.get()), color(color), fontSize(fontSize) {}

  static constexpr const char *metaTableName() { return "sn.TextStyle"; }

  Font *font() const { return fontRef; }
  Ptr<Font> fontPtr;
  Font *fontRef = nullptr;
  Color color = Palette::white();
  float fontSize = 32.0f;
};

class TextTransform {
public:
  TextTransform() = default;
  TextTransform(const Vec2 &position, float angle = 0.0f,
                Pivot pivot = Pivot::TopLeft)
      : position(position), angle(angle), pivot(pivot) {}

  static constexpr const char *metaTableName() { return "sn.TextTransform"; }

  Vec2 position;
  float angle = 0.0f;
  Pivot pivot = Pivot::TopLeft;
};
} // namespace sinen

#endif
