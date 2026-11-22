#include "luaapi.hpp"
#include <math/vec2.hpp>

namespace sinen {
void registerVec2(luau::table &sn) {
  engineClass<Vec2>(sn, Vec2::metaTableName())
      .static_function(
          "new", luau::overload([] { return Vec2(0.0f); },
                                [](float value) { return Vec2(value); },
                                [](float x, float y) { return Vec2(x, y); }))
      .field<&Vec2::x>("x")
      .field<&Vec2::y>("y")
      .metamethod("__add", [](const Vec2 &a, const Vec2 &b) { return a + b; })
      .metamethod("__sub", [](const Vec2 &a, const Vec2 &b) { return a - b; })
      .metamethod("__mul", [](const Vec2 &a, const Vec2 &b) { return a * b; })
      .metamethod("__div", [](const Vec2 &a, const Vec2 &b) { return a / b; })
      .metamethod<&Vec2::tableString>("__tostring")
      .method("copy", [](const Vec2 &v) { return v; })
      .method<&Vec2::length>("length");
}
} // namespace sinen
