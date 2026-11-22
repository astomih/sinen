#include "luaapi.hpp"
#include <math/color/color.hpp>

namespace sinen {
void registerColor(lua_State *L) {
  engineClass<Color>(L, Color::metaTableName())
      .static_function(
          "new",
          luau::overload(
              [] { return Color(0.0f); },
              [](float value) { return Color(value); },
              [](float value, float alpha) { return Color(value, alpha); },
              [](float r, float g, float b, std::optional<float> a) {
                return Color(r, g, b, a.value_or(1.0f));
              }))
      .field("r", &Color::r)
      .field("g", &Color::g)
      .field("b", &Color::b)
      .field("a", &Color::a)
      .metamethod("__tostring",
                  [](const Color &color) { return color.tableString(); });
}
} // namespace sinen
