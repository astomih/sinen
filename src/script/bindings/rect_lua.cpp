#include "luaapi.hpp"
#include <math/geometry/rect.hpp>

namespace sinen {
void registerRect(luau::table &sn) {
  auto binding = engineClass<Rect>(sn, Rect::metaTableName());

  binding.static_function(
      "new",
      luau::overload(
          [] { return Rect(); },
          [](float x, float y, float w, float h) { return Rect(x, y, w, h); },
          [](const Vec2 &position, const Vec2 &size) {
            return Rect(position, size);
          },
          [](Pivot pivot, float x, float y, float w, float h) {
            return Rect(pivot, x, y, w, h);
          },
          [](Pivot pivot, const Vec2 &position, const Vec2 &size) {
            return Rect(pivot, position, size);
          }));

  binding.field<&Rect::x>("x")
      .field<&Rect::y>("y")
      .field<&Rect::width>("width")
      .field<&Rect::height>("height")
      .method<&Rect::intersectsRect>("intersectsRect")
      .method<&Rect::topLeft>("topLeft")
      .method<&Rect::topCenter>("topCenter")
      .method<&Rect::topRight>("topRight")
      .method<&Rect::left>("left")
      .method<&Rect::center>("center")
      .method<&Rect::right>("right")
      .method<&Rect::bottomLeft>("bottomLeft")
      .method<&Rect::bottomCenter>("bottomCenter")
      .method<&Rect::bottomRight>("bottomRight")
      .method<&Rect::position>("position")
      .method<&Rect::size>("size")
      .method<&Rect::positionfromPivot>("positionFromPivot");
}
} // namespace sinen
