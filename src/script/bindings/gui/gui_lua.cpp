#include <script/luaapi.hpp>
#include <gui/gui.hpp>
#include <graphics/graphics.hpp>
#include <platform/input/mouse.hpp>
#include <platform/window/window.hpp>

namespace sinen {
static int lGuiSetFont(lua_State *L) {
  auto &font = udPtr<Font>(L, 1);
  Gui::setFont(font);
  return 0;
}

static int lGuiSetFontSize(lua_State *L) {
  Gui::setFontSize(static_cast<float>(luaL_checknumber(L, 1)));
  return 0;
}

static int lGuiSetThemeColor(lua_State *L) {
  auto &background = udValue<Color>(L, 1);
  auto &hover = udValue<Color>(L, 2);
  auto &active = udValue<Color>(L, 3);
  auto &text = udValue<Color>(L, 4);
  auto &accent = udValue<Color>(L, 5);
  Gui::setThemeColor(background, hover, active, text, accent);
  return 0;
}

static int lGuiLabel(lua_State *L) {
  const char *text = luaL_checkstring(L, 1);
  auto &position = udValue<Vec2>(L, 2);
  Color color = Gui::textColor();
  if (lua_gettop(L) >= 3 && !lua_isnoneornil(L, 3)) {
    color = udValue<Color>(L, 3);
  }
  const float fontSize =
      static_cast<float>(luaL_optnumber(L, 4, Gui::fontSize()));
  Gui::label(StringView(text), position, color, fontSize);
  return 0;
}

static int lGuiButton(lua_State *L) {
  const char *text = luaL_checkstring(L, 1);
  auto &rect = udValue<Rect>(L, 2);
  lua_pushboolean(L, Gui::button(StringView(text), rect));
  return 1;
}

static int lGuiCheckbox(lua_State *L) {
  const char *text = luaL_checkstring(L, 1);
  const bool checked = lua_toboolean(L, 2) != 0;
  auto &rect = udValue<Rect>(L, 3);
  lua_pushboolean(L, Gui::checkbox(StringView(text), checked, rect));
  return 1;
}

static int lGuiSliderFloat(lua_State *L) {
  const char *text = luaL_checkstring(L, 1);
  const float value = static_cast<float>(luaL_checknumber(L, 2));
  const float min = static_cast<float>(luaL_checknumber(L, 3));
  const float max = static_cast<float>(luaL_checknumber(L, 4));
  auto &rect = udValue<Rect>(L, 5);
  lua_pushnumber(L, Gui::sliderFloat(StringView(text), value, min, max, rect));
  return 1;
}

static int lGuiScrollVertical(lua_State *L) {
  const float scroll = static_cast<float>(luaL_checknumber(L, 1));
  auto &viewport = udValue<Rect>(L, 2);
  const float contentHeight = static_cast<float>(luaL_checknumber(L, 3));
  const float wheelStep = static_cast<float>(luaL_optnumber(L, 4, 48.0));
  lua_pushnumber(
      L, Gui::scrollVertical(scroll, viewport, contentHeight, wheelStep));
  return 1;
}

void registerGui(lua_State *L) {
  pushSnNamed(L, "Gui");
  luaPushcfunction2(L, lGuiSetFont);
  lua_setfield(L, -2, "setFont");
  luaPushcfunction2(L, lGuiSetFontSize);
  lua_setfield(L, -2, "setFontSize");
  luaPushcfunction2(L, lGuiSetThemeColor);
  lua_setfield(L, -2, "setThemeColor");
  luaPushcfunction2(L, lGuiLabel);
  lua_setfield(L, -2, "label");
  luaPushcfunction2(L, lGuiButton);
  lua_setfield(L, -2, "button");
  luaPushcfunction2(L, lGuiCheckbox);
  lua_setfield(L, -2, "checkbox");
  luaPushcfunction2(L, lGuiSliderFloat);
  lua_setfield(L, -2, "sliderFloat");
  luaPushcfunction2(L, lGuiScrollVertical);
  lua_setfield(L, -2, "scrollVertical");
  lua_pop(L, 1);
}
} // namespace sinen
