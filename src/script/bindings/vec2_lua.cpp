#include "luaapi.hpp"
#include <math/vec2.hpp>

namespace sinen {
void registerVec2(lua_State *L) {
  engineClass<Vec2>(L, Vec2::metaTableName())
      .static_function(
          "new", luau::overload([] { return Vec2(0.0f); },
                                [](float value) { return Vec2(value); },
                                [](float x, float y) { return Vec2(x, y); }))
      .field("x", &Vec2::x)
      .field("y", &Vec2::y)
      .metamethod("__add", [](const Vec2 &a, const Vec2 &b) { return a + b; })
      .metamethod("__sub", [](const Vec2 &a, const Vec2 &b) { return a - b; })
      .metamethod("__mul", [](const Vec2 &a, const Vec2 &b) { return a * b; })
      .metamethod("__div", [](const Vec2 &a, const Vec2 &b) { return a / b; })
      .metamethod("__tostring", [](const Vec2 &v) { return v.tableString(); })
      .method("copy", [](const Vec2 &v) { return v; })
      .method("length", [](const Vec2 &v) { return v.length(); });
}
} // namespace sinen
