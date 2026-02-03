#include <math/vec2.hpp>
#include <script/luaapi.hpp>

#include <imgui.h>

namespace sinen {
static int lImGuiBegin(lua_State *L) {
  int n = lua_gettop(L);
  const char *name = luaL_checkstring(L, 1);
  if (n >= 2) {
    int flags = static_cast<int>(luaL_checkinteger(L, 2));
    ImGui::Begin(name, nullptr, flags);
    return 0;
  }
  lua_pushboolean(L, ImGui::Begin(name));
  return 1;
}
static int lImGuiEnd(lua_State *L) {
  (void)L;
  ImGui::End();
  return 0;
}
static int lImGuiButton(lua_State *L) {
  const char *name = luaL_checkstring(L, 1);
  lua_pushboolean(L, ImGui::Button(name));
  return 1;
}
static int lImGuiText(lua_State *L) {
  const char *text = luaL_checkstring(L, 1);
  ImGui::Text("%s", text);
  return 0;
}
static int lImGuiSetNextWindowPos(lua_State *L) {
  auto &pos = udValue<Vec2>(L, 1);
  ImGui::SetNextWindowPos({pos.x, pos.y});
  return 0;
}
static int lImGuiSetNextWindowSize(lua_State *L) {
  auto &size = udValue<Vec2>(L, 1);
  ImGui::SetNextWindowSize({size.x, size.y});
  return 0;
}
void registerImGui(lua_State *L) {
  pushSnNamed(L, "ImGui");
  luaPushcfunction2(L, lImGuiBegin);
  lua_setfield(L, -2, "Begin");
  luaPushcfunction2(L, lImGuiEnd);
  lua_setfield(L, -2, "End");
  luaPushcfunction2(L, lImGuiButton);
  lua_setfield(L, -2, "button");
  luaPushcfunction2(L, lImGuiText);
  lua_setfield(L, -2, "text");
  luaPushcfunction2(L, lImGuiSetNextWindowPos);
  lua_setfield(L, -2, "setNextWindowPos");
  luaPushcfunction2(L, lImGuiSetNextWindowSize);
  lua_setfield(L, -2, "setNextWindowSize");

  lua_newtable(L);
  lua_pushvalue(L, -1);
  lua_setfield(L, -3, "WindowFlags");
  lua_pushinteger(L, ImGuiWindowFlags_None);
  lua_setfield(L, -2, "None");
  lua_pushinteger(L, ImGuiWindowFlags_NoTitleBar);
  lua_setfield(L, -2, "NoTitleBar");
  lua_pushinteger(L, ImGuiWindowFlags_NoResize);
  lua_setfield(L, -2, "NoResize");
  lua_pushinteger(L, ImGuiWindowFlags_NoMove);
  lua_setfield(L, -2, "NoMove");
  lua_pushinteger(L, ImGuiWindowFlags_NoScrollbar);
  lua_setfield(L, -2, "NoScrollBar");
  lua_pushinteger(L, ImGuiWindowFlags_NoScrollWithMouse);
  lua_setfield(L, -2, "NoScrollWithMouse");
  lua_pushinteger(L, ImGuiWindowFlags_NoCollapse);
  lua_setfield(L, -2, "NoCollapse");
  lua_pushinteger(L, ImGuiWindowFlags_AlwaysAutoResize);
  lua_setfield(L, -2, "AlwaysAutoResize");
  lua_pushinteger(L, ImGuiWindowFlags_NoBackground);
  lua_setfield(L, -2, "NoBackground");
  lua_pushinteger(L, ImGuiWindowFlags_NoSavedSettings);
  lua_setfield(L, -2, "NoSavedSettings");
  lua_pushinteger(L, ImGuiWindowFlags_NoMouseInputs);
  lua_setfield(L, -2, "NoMouseInputs");
  lua_pushinteger(L, ImGuiWindowFlags_MenuBar);
  lua_setfield(L, -2, "MenuBar");
  lua_pushinteger(L, ImGuiWindowFlags_HorizontalScrollbar);
  lua_setfield(L, -2, "HorizontalScrollbar");
  lua_pushinteger(L, ImGuiWindowFlags_NoFocusOnAppearing);
  lua_setfield(L, -2, "NoFocusOnAppearing");
  lua_pushinteger(L, ImGuiWindowFlags_NoBringToFrontOnFocus);
  lua_setfield(L, -2, "NoBringToFrontOnFocus");
  lua_pushinteger(L, ImGuiWindowFlags_AlwaysVerticalScrollbar);
  lua_setfield(L, -2, "AlwaysVerticalScrollbar");
  lua_pushinteger(L, ImGuiWindowFlags_AlwaysHorizontalScrollbar);
  lua_setfield(L, -2, "AlwaysHorizontalScrollbar");
  lua_pushinteger(L, ImGuiWindowFlags_NoNavInputs);
  lua_setfield(L, -2, "NoNavInputs");
  lua_pushinteger(L, ImGuiWindowFlags_NoNavFocus);
  lua_setfield(L, -2, "NoNavFocus");
  lua_pushinteger(L, ImGuiWindowFlags_UnsavedDocument);
  lua_setfield(L, -2, "UnsavedDocument");
  lua_pushinteger(L, ImGuiWindowFlags_NoNav);
  lua_setfield(L, -2, "NoNav");
  lua_pushinteger(L, ImGuiWindowFlags_NoDecoration);
  lua_setfield(L, -2, "NoDecoration");
  lua_pushinteger(L, ImGuiWindowFlags_NoInputs);
  lua_setfield(L, -2, "NoInputs");
  lua_pop(L, 2); // WindowFlags + ImGui
}
} // namespace sinen