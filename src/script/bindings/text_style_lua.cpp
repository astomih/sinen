#include "luaapi.hpp"
#include <graphics/text_style.hpp>

namespace sinen {
void registerTextStyle(luau::table &sn) {
  engineClass<TextStyle>(sn, TextStyle::metaTableName())
      .static_function(
          "new",
          luau::overload(
              [](const Ptr<Font> &font, std::optional<Color> color,
                 std::optional<float> size) {
                return TextStyle(font, color.value_or(Palette::white()),
                                 size.value_or(32.0f));
              },
              [](const Ptr<Font> &font, std::optional<float> size,
                 std::optional<float> overrideSize) {
                return TextStyle(font, Palette::white(),
                                 overrideSize.value_or(size.value_or(32.0f)));
              }))
      .property(
          "font",
          [](const TextStyle &style) -> std::optional<Ptr<Font>> {
            if (style.fontPtr)
              return style.fontPtr;
            return std::nullopt;
          },
          [](TextStyle &style, Ptr<Font> font) {
            style.fontPtr = std::move(font);
            style.fontRef = style.fontPtr.get();
          })
      .field<&TextStyle::color>("color")
      .field<&TextStyle::fontSize>("fontSize");

  engineClass<TextTransform>(sn, TextTransform::metaTableName())
      .static_function("new",
                       [](const Vec2 &position, std::optional<float> angle,
                          std::optional<Pivot> pivot) {
                         return TextTransform(position, angle.value_or(0.0f),
                                              pivot.value_or(Pivot::TopLeft));
                       })
      .field<&TextTransform::position>("position")
      .field<&TextTransform::angle>("angle")
      .field<&TextTransform::pivot>("pivot");
}
} // namespace sinen
