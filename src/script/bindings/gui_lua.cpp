#include "luaapi.hpp"
#include <graphics/graphics.hpp>
#include <graphics/gui/gui.hpp>
#include <platform/input/mouse.hpp>
#include <platform/window/window.hpp>

namespace sinen {
static auto lGuiSetFont(Ptr<Font> &font) {
  Gui::setFont(font);
  return;
}

static int lGuiSetFontSize(lua_State *L) {
  Gui::setFontSize(static_cast<float>(luaL_checknumber(L, 1)));
  return 0;
}

static auto lGuiSetThemeColor(Color &background, Color &hover, Color &active,
                              Color &text, Color &accent) {
  Gui::setThemeColor(background, hover, active, text, accent);
  return;
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

static auto lGuiButton(const char *text, Rect &rect) {
  return Gui::button(StringView(text), rect);
}

static int lGuiCheckbox(lua_State *L) {
  const char *text = luaL_checkstring(L, 1);
  const bool checked = lua_toboolean(L, 2) != 0;
  auto &rect = udValue<Rect>(L, 3);
  lua_pushboolean(L, Gui::checkbox(StringView(text), checked, rect));
  return 1;
}

static auto lGuiSliderFloat(const char *text, const float value,
                            const float min, const float max, Rect &rect) {
  return Gui::sliderFloat(StringView(text), value, min, max, rect);
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
  luau::table(L, -1).function("setFont", lGuiSetFont);
  luau::table(L, -1).raw_function("setFontSize", lGuiSetFontSize);
  luau::table(L, -1).function("setThemeColor", lGuiSetThemeColor);
  luau::table(L, -1).raw_function("label", lGuiLabel);
  luau::table(L, -1).function("button", lGuiButton);
  luau::table(L, -1).raw_function("checkbox", lGuiCheckbox);
  luau::table(L, -1).function("sliderFloat", lGuiSliderFloat);
  luau::table(L, -1).raw_function("scrollVertical", lGuiScrollVertical);
  lua_pop(L, 1);
}
} // namespace sinen
