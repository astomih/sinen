#include <core/data/array.hpp>
#include <core/data/hashmap.hpp>
#include <gpu/gpu.hpp>
#include <graphics/font/font.hpp>
#include <graphics/texture/texture.hpp>
#include <math/color/color.hpp>
#include <math/geometry/mesh.hpp>
#include <math/math.hpp>
#include <platform/io/asset_reader.hpp>
#include <script/luaapi.hpp>

#include <cstring>

namespace sinen {
static FontMethod lFontMethod(lua_State *L, int index) {
  if (index > lua_gettop(L) || lua_isnoneornil(L, index)) {
    return FontMethod::MSDF;
  }
  if (lua_isstring(L, index)) {
    const char *method = luaL_checkstring(L, index);
    if (std::strcmp(method, "msdf") == 0 || std::strcmp(method, "MSDF") == 0) {
      return FontMethod::MSDF;
    }
    return FontMethod::Bitmap;
  }
  return static_cast<FontMethod>(luaL_checkinteger(L, index));
}

static bool lIsFontMethodString(lua_State *L, int index) {
  if (!lua_isstring(L, index)) {
    return false;
  }
  const char *method = lua_tostring(L, index);
  return std::strcmp(method, "bitmap") == 0 ||
         std::strcmp(method, "Bitmap") == 0 ||
         std::strcmp(method, "msdf") == 0 || std::strcmp(method, "MSDF") == 0;
}

static int lFontNew(lua_State *L) {
  udPushPtr<Font>(L, Font::create());
  return 1;
}
static int lFontLoad(lua_State *L) {
  auto &font = udPtr<Font>(L, 1);
  int n = lua_gettop(L);
  int point = static_cast<int>(luaL_checkinteger(L, 2));
  if (n == 2) {
    lua_pushboolean(L, font->load(point));
    return 1;
  }
  if (n == 3 && (lua_isnumber(L, 3) || lIsFontMethodString(L, 3))) {
    lua_pushboolean(L, font->load(point, lFontMethod(L, 3)));
    return 1;
  }
  if (lua_isstring(L, 3)) {
    const char *path = luaL_checkstring(L, 3);
    lua_pushboolean(L, font->load(point, StringView(path), lFontMethod(L, 4)));
    return 1;
  }
  auto &buf = udValue<Buffer>(L, 3);
  lua_pushboolean(L, font->load(point, buf, lFontMethod(L, 4)));
  return 1;
}
static int lFontResize(lua_State *L) {
  auto &font = udPtr<Font>(L, 1);
  int point = static_cast<int>(luaL_checkinteger(L, 2));
  font->resize(point);
  return 0;
}
static int lFontRegion(lua_State *L) {
  auto &font = udPtr<Font>(L, 1);
  const char *text = luaL_checkstring(L, 2);
  int fontSize = static_cast<int>(luaL_checkinteger(L, 3));
  Pivot pivot = static_cast<Pivot>(luaL_checkinteger(L, 4));
  auto &vec = udValue<Vec2>(L, 5);
  udNewOwned<Rect>(L, font->region(StringView(text), fontSize, pivot, vec));
  return 1;
}
void registerFont(lua_State *L) {

  luaL_newmetatable(L, Font::metaTableName());
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lFontLoad);
  lua_setfield(L, -2, "load");
  luaPushcfunction2(L, lFontResize);
  lua_setfield(L, -2, "resize");
  luaPushcfunction2(L, lFontRegion);
  lua_setfield(L, -2, "region");
  lua_pop(L, 1);

  pushSnNamed(L, "Font");
  luaPushcfunction2(L, lFontNew);
  lua_setfield(L, -2, "new");
  lua_pushinteger(L, static_cast<lua_Integer>(FontMethod::Bitmap));
  lua_setfield(L, -2, "Bitmap");
  lua_pushinteger(L, static_cast<lua_Integer>(FontMethod::MSDF));
  lua_setfield(L, -2, "MSDF");
  lua_pop(L, 1);
}

} // namespace sinen
