#include "luaapi.hpp"

#include <core/data/array.hpp>
#include <core/data/hashmap.hpp>
#include <gpu/gpu.hpp>
#include <graphics/font/font.hpp>
#include <graphics/texture/texture.hpp>
#include <math/color/color.hpp>
#include <math/geometry/mesh.hpp>
#include <math/math.hpp>
#include <platform/io/asset_reader.hpp>

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
  const int n = lua_gettop(L);
  if (n < 1 || n > 3) {
    return luaLError2(
        L, "sn.Font.new expects size, optional source, and optional method");
  }
  const int point = static_cast<int>(luaL_checkinteger(L, 1));
  if (point <= 0) {
    return luaLError2(L, "sn.Font.new size must be greater than zero");
  }

  auto font = Font::create();
  bool loaded = false;
  if (n == 1 || (n == 2 && (lua_isnumber(L, 2) || lIsFontMethodString(L, 2)))) {
    loaded = font->load(point, lFontMethod(L, 2));
  } else if (lua_isstring(L, 2)) {
    const char *path = luaL_checkstring(L, 2);
    if (!AssetReader::exists(path)) {
      font.reset();
      return luaLError2(L, "sn.Font.new asset not found: %s", path);
    }
    loaded = font->load(point, StringView(path), lFontMethod(L, 3));
  } else {
    auto &buffer = udValue<Buffer>(L, 2);
    loaded = buffer.size() > 0 && font->load(point, buffer, lFontMethod(L, 3));
  }
  if (!loaded) {
    font.reset();
    return luaLError2(L, "sn.Font.new failed to load the font");
  }
  udPushPtr<Font>(L, std::move(font));
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
