#include <cstring>

#include <graphics/text_style.hpp>
#include <script/luaapi.hpp>

namespace sinen {
static int lTextStyleNew(lua_State *L) {
  auto &font = udPtr<Font>(L, 1);
  Color color = Palette::white();
  float fontSize = 32.0f;

  if (lua_gettop(L) >= 2 && !lua_isnoneornil(L, 2)) {
    if (auto *c = udValueOrNull<Color>(L, 2)) {
      color = *c;
    } else {
      fontSize = static_cast<float>(luaL_checknumber(L, 2));
    }
  }
  if (lua_gettop(L) >= 3 && !lua_isnoneornil(L, 3)) {
    fontSize = static_cast<float>(luaL_checknumber(L, 3));
  }

  udNewOwned<TextStyle>(L, TextStyle(font, color, fontSize));
  return 1;
}

static int lTextStyleIndex(lua_State *L) {
  auto &style = udValue<TextStyle>(L, 1);
  const char *k = luaL_checkstring(L, 2);
  if (std::strcmp(k, "font") == 0) {
    if (style.fontPtr) {
      udPushPtr<Font>(L, style.fontPtr);
    } else {
      lua_pushnil(L);
    }
    return 1;
  }
  if (std::strcmp(k, "color") == 0) {
    udNewOwned<Color>(L, style.color);
    return 1;
  }
  if (std::strcmp(k, "fontSize") == 0) {
    lua_pushnumber(L, style.fontSize);
    return 1;
  }
  luaL_getmetatable(L, TextStyle::metaTableName());
  lua_pushvalue(L, 2);
  lua_rawget(L, -2);
  return 1;
}

static int lTextStyleNewindex(lua_State *L) {
  auto &style = udValue<TextStyle>(L, 1);
  const char *k = luaL_checkstring(L, 2);
  if (std::strcmp(k, "font") == 0) {
    style.fontPtr = udPtr<Font>(L, 3);
    style.fontRef = style.fontPtr.get();
    return 0;
  }
  if (std::strcmp(k, "color") == 0) {
    style.color = udValue<Color>(L, 3);
    return 0;
  }
  if (std::strcmp(k, "fontSize") == 0) {
    style.fontSize = static_cast<float>(luaL_checknumber(L, 3));
    return 0;
  }
  return luaLError2(L, "sn.TextStyle: invalid field '%s'", k);
}

static int lTextTransformNew(lua_State *L) {
  auto &position = udValue<Vec2>(L, 1);
  float angle = static_cast<float>(luaL_optnumber(L, 2, 0.0));
  Pivot pivot = Pivot::TopLeft;
  if (lua_gettop(L) >= 3 && !lua_isnoneornil(L, 3)) {
    pivot = static_cast<Pivot>(luaL_checkinteger(L, 3));
  }
  udNewOwned<TextTransform>(L, TextTransform(position, angle, pivot));
  return 1;
}

static int lTextTransformIndex(lua_State *L) {
  auto &transform = udValue<TextTransform>(L, 1);
  const char *k = luaL_checkstring(L, 2);
  if (std::strcmp(k, "position") == 0) {
    udNewOwned<Vec2>(L, transform.position);
    return 1;
  }
  if (std::strcmp(k, "angle") == 0) {
    lua_pushnumber(L, transform.angle);
    return 1;
  }
  if (std::strcmp(k, "pivot") == 0) {
    lua_pushinteger(L, static_cast<lua_Integer>(transform.pivot));
    return 1;
  }
  luaL_getmetatable(L, TextTransform::metaTableName());
  lua_pushvalue(L, 2);
  lua_rawget(L, -2);
  return 1;
}

static int lTextTransformNewindex(lua_State *L) {
  auto &transform = udValue<TextTransform>(L, 1);
  const char *k = luaL_checkstring(L, 2);
  if (std::strcmp(k, "position") == 0) {
    transform.position = udValue<Vec2>(L, 3);
    return 0;
  }
  if (std::strcmp(k, "angle") == 0) {
    transform.angle = static_cast<float>(luaL_checknumber(L, 3));
    return 0;
  }
  if (std::strcmp(k, "pivot") == 0) {
    transform.pivot = static_cast<Pivot>(luaL_checkinteger(L, 3));
    return 0;
  }
  return luaLError2(L, "sn.TextTransform: invalid field '%s'", k);
}

void registerTextStyle(lua_State *L) {
  luaL_newmetatable(L, TextStyle::metaTableName());
  luaPushcfunction2(L, udGc<TextStyle>);
  lua_setfield(L, -2, "__gc");
  luaPushcfunction2(L, lTextStyleIndex);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lTextStyleNewindex);
  lua_setfield(L, -2, "__newindex");
  lua_pop(L, 1);

  pushSnNamed(L, "TextStyle");
  luaPushcfunction2(L, lTextStyleNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);

  luaL_newmetatable(L, TextTransform::metaTableName());
  luaPushcfunction2(L, udGc<TextTransform>);
  lua_setfield(L, -2, "__gc");
  luaPushcfunction2(L, lTextTransformIndex);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lTextTransformNewindex);
  lua_setfield(L, -2, "__newindex");
  lua_pop(L, 1);

  pushSnNamed(L, "TextTransform");
  luaPushcfunction2(L, lTextTransformNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}
} // namespace sinen
