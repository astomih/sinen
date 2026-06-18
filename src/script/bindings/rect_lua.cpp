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

static int lRectIndex(lua_State *L) {
  auto &r = udValue<Rect>(L, 1);
  const char *k = luaL_checkstring(L, 2);
  if (std::strcmp(k, "x") == 0) {
    lua_pushnumber(L, r.x);
    return 1;
  }
  if (std::strcmp(k, "y") == 0) {
    lua_pushnumber(L, r.y);
    return 1;
  }
  if (std::strcmp(k, "width") == 0) {
    lua_pushnumber(L, r.width);
    return 1;
  }
  if (std::strcmp(k, "height") == 0) {
    lua_pushnumber(L, r.height);
    return 1;
  }
  luaL_getmetatable(L, Rect::metaTableName());
  lua_pushvalue(L, 2);
  lua_rawget(L, -2);
  return 1;
}
static int lRectNewindex(lua_State *L) {
  auto &r = udValue<Rect>(L, 1);
  const char *k = luaL_checkstring(L, 2);
  float v = static_cast<float>(luaL_checknumber(L, 3));
  if (std::strcmp(k, "x") == 0) {
    r.x = v;
    return 0;
  }
  if (std::strcmp(k, "y") == 0) {
    r.y = v;
    return 0;
  }
  if (std::strcmp(k, "width") == 0) {
    r.width = v;
    return 0;
  }
  if (std::strcmp(k, "height") == 0) {
    r.height = v;
    return 0;
  }
  return luaLError2(L, "sn.Rect: invalid field '%s'", k);
}
static int lRectIntersectsRect(lua_State *L) {
  auto &a = udValue<Rect>(L, 1);
  auto &b = udValue<Rect>(L, 2);
  lua_pushboolean(L, a.intersectsRect(b));
  return 1;
}
void registerRect(lua_State *L) {
  luaL_newmetatable(L, Rect::metaTableName());
  luaPushcfunction2(L, udGc<Rect>);
  lua_setfield(L, -2, "__gc");
  luaPushcfunction2(L, lRectIndex);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lRectNewindex);
  lua_setfield(L, -2, "__newindex");
  luaPushcfunction2(L, lRectIntersectsRect);
  lua_setfield(L, -2, "intersectsRect");

  luaPushcfunction2(L, [](lua_State *L) -> int {
    auto &r = udValue<Rect>(L, 1);
    udNewOwned<Vec2>(L, r.topLeft());
    return 1;
  });
  lua_setfield(L, -2, "topLeft");

  luaPushcfunction2(L, [](lua_State *L) -> int {
    auto &r = udValue<Rect>(L, 1);
    udNewOwned<Vec2>(L, r.topCenter());
    return 1;
  });
  lua_setfield(L, -2, "topCenter");

  luaPushcfunction2(L, [](lua_State *L) -> int {
    auto &r = udValue<Rect>(L, 1);
    udNewOwned<Vec2>(L, r.topRight());
    return 1;
  });
  lua_setfield(L, -2, "topRight");

  luaPushcfunction2(L, [](lua_State *L) -> int {
    auto &r = udValue<Rect>(L, 1);
    udNewOwned<Vec2>(L, r.left());
    return 1;
  });
  lua_setfield(L, -2, "left");

  luaPushcfunction2(L, [](lua_State *L) -> int {
    auto &r = udValue<Rect>(L, 1);
    udNewOwned<Vec2>(L, r.center());
    return 1;
  });
  lua_setfield(L, -2, "center");

  luaPushcfunction2(L, [](lua_State *L) -> int {
    auto &r = udValue<Rect>(L, 1);
    udNewOwned<Vec2>(L, r.right());
    return 1;
  });
  lua_setfield(L, -2, "right");

  luaPushcfunction2(L, [](lua_State *L) -> int {
    auto &r = udValue<Rect>(L, 1);
    udNewOwned<Vec2>(L, r.bottomLeft());
    return 1;
  });
  lua_setfield(L, -2, "bottomLeft");

  luaPushcfunction2(L, [](lua_State *L) -> int {
    auto &r = udValue<Rect>(L, 1);
    udNewOwned<Vec2>(L, r.bottomCenter());
    return 1;
  });
  lua_setfield(L, -2, "bottomCenter");

  luaPushcfunction2(L, [](lua_State *L) -> int {
    auto &r = udValue<Rect>(L, 1);
    udNewOwned<Vec2>(L, r.bottomRight());
    return 1;
  });
  lua_setfield(L, -2, "bottomRight");

  luaPushcfunction2(L, [](lua_State *L) -> int {
    auto &r = udValue<Rect>(L, 1);
    udNewOwned<Vec2>(L, r.position());
    return 1;
  });
  lua_setfield(L, -2, "position");

  luaPushcfunction2(L, [](lua_State *L) -> int {
    auto &r = udValue<Rect>(L, 1);
    const Pivot pivot = static_cast<Pivot>(luaL_checkinteger(L, 2));
    udNewOwned<Vec2>(L, r.positionfromPivot(pivot));
    return 1;
  });
  lua_setfield(L, -2, "positionFromPivot");

  luaPushcfunction2(L, [](lua_State *L) -> int {
    auto &r = udValue<Rect>(L, 1);
    udNewOwned<Vec2>(L, r.size());
    return 1;
  });
  lua_setfield(L, -2, "size");

  lua_pop(L, 1);

  pushSnNamed(L, "Rect");
  luaPushcfunction2(L, lRectNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}

} // namespace sinen
