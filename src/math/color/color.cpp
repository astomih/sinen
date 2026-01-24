// internal
#include <math/color/color.hpp>
#include <math/color/palette.hpp>

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

#include <asset/script/luaapi.hpp>
namespace sinen {
String Color::tableString() const {
  TablePair p;
  p.emplace_back("r", toStringTrim(r));
  p.emplace_back("g", toStringTrim(g));
  p.emplace_back("b", toStringTrim(b));
  p.emplace_back("a", toStringTrim(a));
  return convert("sn.Color", p, false);
}
static int lColorNew(lua_State *L) {
  int n = lua_gettop(L);
  if (n == 0) {
    udNewOwned<Color>(L, Color(0.0f));
    return 1;
  }
  if (n == 1) {
    float v = static_cast<float>(luaL_checknumber(L, 1));
    udNewOwned<Color>(L, Color(v));
    return 1;
  }
  if (n == 2) {
    float v = static_cast<float>(luaL_checknumber(L, 1));
    float a = static_cast<float>(luaL_checknumber(L, 2));
    udNewOwned<Color>(L, Color(v, a));
    return 1;
  }
  float r = static_cast<float>(luaL_checknumber(L, 1));
  float g = static_cast<float>(luaL_checknumber(L, 2));
  float b = static_cast<float>(luaL_checknumber(L, 3));
  float a = static_cast<float>(luaL_optnumber(L, 4, 1.0));
  udNewOwned<Color>(L, Color(r, g, b, a));
  return 1;
}
static int lColorIndex(lua_State *L) {
  auto &c = udValue<Color>(L, 1);
  const char *k = luaL_checkstring(L, 2);
  if (std::strcmp(k, "r") == 0) {
    lua_pushnumber(L, c.r);
    return 1;
  }
  if (std::strcmp(k, "g") == 0) {
    lua_pushnumber(L, c.g);
    return 1;
  }
  if (std::strcmp(k, "b") == 0) {
    lua_pushnumber(L, c.b);
    return 1;
  }
  if (std::strcmp(k, "a") == 0) {
    lua_pushnumber(L, c.a);
    return 1;
  }
  luaL_getmetatable(L, Color::metaTableName());
  lua_pushvalue(L, 2);
  lua_rawget(L, -2);
  return 1;
}
static int lColorNewindex(lua_State *L) {
  auto &c = udValue<Color>(L, 1);
  const char *k = luaL_checkstring(L, 2);
  float value = static_cast<float>(luaL_checknumber(L, 3));
  if (std::strcmp(k, "r") == 0) {
    c.r = value;
    return 0;
  }
  if (std::strcmp(k, "g") == 0) {
    c.g = value;
    return 0;
  }
  if (std::strcmp(k, "b") == 0) {
    c.b = value;
    return 0;
  }
  if (std::strcmp(k, "a") == 0) {
    c.a = value;
    return 0;
  }
  return luaLError2(L, "sn.Color: invalid field '%s'", k);
}
static int lColorTostring(lua_State *L) {
  auto &c = udValue<Color>(L, 1);
  String s = c.tableString();
  lua_pushlstring(L, s.data(), s.size());
  return 1;
}
void registerColor(lua_State *L) {
  luaL_newmetatable(L, Color::metaTableName());
  luaPushcfunction2(L, udGc<Color>);
  lua_setfield(L, -2, "__gc");
  luaPushcfunction2(L, lColorIndex);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lColorNewindex);
  lua_setfield(L, -2, "__newindex");
  luaPushcfunction2(L, lColorTostring);
  lua_setfield(L, -2, "__tostring");
  lua_pop(L, 1);

  pushSnNamed(L, "Color");
  luaPushcfunction2(L, lColorNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}
} // namespace sinen