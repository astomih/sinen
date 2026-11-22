#include "luaapi.hpp"
#include <math/geometry/rect.hpp>

namespace sinen {
static int lRectNew(lua_State *L) {
  int n = lua_gettop(L);
  if (n == 0) {
    udNewOwned<Rect>(L, Rect());
    return 1;
  }
  if (n == 4) {
    float x = static_cast<float>(luaL_checknumber(L, 1));
    float y = static_cast<float>(luaL_checknumber(L, 2));
    float w = static_cast<float>(luaL_checknumber(L, 3));
    float h = static_cast<float>(luaL_checknumber(L, 4));
    udNewOwned<Rect>(L, Rect(x, y, w, h));
    return 1;
  }
  if (n == 2) {
    auto &pos = udValue<Vec2>(L, 1);
    auto &size = udValue<Vec2>(L, 2);
    udNewOwned<Rect>(L, Rect(pos, size));
    return 1;
  }
  if (n == 5) {
    Pivot pivot = static_cast<Pivot>(luaL_checkinteger(L, 1));
    float x = static_cast<float>(luaL_checknumber(L, 2));
    float y = static_cast<float>(luaL_checknumber(L, 3));
    float w = static_cast<float>(luaL_checknumber(L, 4));
    float h = static_cast<float>(luaL_checknumber(L, 5));
    udNewOwned<Rect>(L, Rect(pivot, x, y, w, h));
    return 1;
  }
  if (n == 3) {
    Pivot pivot = static_cast<Pivot>(luaL_checkinteger(L, 1));
    auto &pos = udValue<Vec2>(L, 2);
    auto &size = udValue<Vec2>(L, 3);
    udNewOwned<Rect>(L, Rect(pivot, pos, size));
    return 1;
  }
  return luaLError2(L, "sn.Rect.new: invalid arguments");
}

void registerRect(lua_State *L) {
  auto binding = engineClass<Rect>(L, Rect::metaTableName());
  pushSnNamed(L, "Rect");
  luau::table(L, -1).raw_function("new", lRectNew);
  lua_pop(L, 1);
  binding.field("x", &Rect::x)
      .field("y", &Rect::y)
      .field("width", &Rect::width)
      .field("height", &Rect::height)
      .method("intersectsRect", &Rect::intersectsRect)
      .method("topLeft", &Rect::topLeft)
      .method("topCenter", &Rect::topCenter)
      .method("topRight", &Rect::topRight)
      .method("left", &Rect::left)
      .method("center", &Rect::center)
      .method("right", &Rect::right)
      .method("bottomLeft", &Rect::bottomLeft)
      .method("bottomCenter", &Rect::bottomCenter)
      .method("bottomRight", &Rect::bottomRight)
      .method("position", &Rect::position)
      .method("size", &Rect::size)
      .method("positionFromPivot", &Rect::positionfromPivot);
}
} // namespace sinen
