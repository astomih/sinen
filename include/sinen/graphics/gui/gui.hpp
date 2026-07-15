#ifndef SINEN_GUI_HPP
#define SINEN_GUI_HPP

#include <core/data/string.hpp>
#include <graphics/font/font.hpp>
#include <math/color/color.hpp>
#include <math/geometry/rect.hpp>
#include <math/vector.hpp>

namespace sinen {
class Gui {
public:
  static void shutdown();
  static void newFrame();
  static void setFont(const Ptr<Font> &font);
  static void setFontSize(float size);
  static float fontSize();
  static void setThemeColor(const Color &background, const Color &hover,
                            const Color &active, const Color &text,
                            const Color &accent);
  static Color textColor();
  static void label(StringView text, const Vec2 &position,
                    const Color &color = Color(1.0f), float fontSize = 0.0f);
  static bool button(StringView text, const Rect &rect);
  static bool checkbox(StringView text, bool checked, const Rect &rect);
  static float sliderFloat(StringView text, float value, float min, float max,
                           const Rect &rect);
  static float scrollVertical(float scroll, const Rect &viewport,
                              float contentHeight, float wheelStep = 48.0f);
};
} // namespace sinen

#endif // SINEN_GUI_HPP
