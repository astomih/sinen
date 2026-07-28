#include "luaapi.hpp"
#include <core/def/macro.hpp>
#include <core/parser/json.hpp>
#include <platform/input/keyboard.hpp>
#include <platform/io/filesystem.hpp>
#include <platform/window/window.hpp>

namespace sinen {
static int lWindowGetName(lua_State *L) {
  auto name = Window::name();
  lua_pushlstring(L, name.data(), name.size());
  return 1;
}
static int lWindowSize(lua_State *L) {
  udNewOwned<Vec2>(L, Window::size());
  return 1;
}
static int lWindowHalf(lua_State *L) {
  udNewOwned<Vec2>(L, Window::half());
  return 1;
}
static int lWindowResize(lua_State *L) {
  auto &size = udValue<Vec2>(L, 1);
  Window::resize(size);
  return 0;
}
static int lWindowSetFullscreen(lua_State *L) {
  bool fs = lua_toboolean(L, 1) != 0;
  Window::setFullscreen(fs);
  return 0;
}
static int lWindowRename(lua_State *L) {
  const char *name = luaL_checkstring(L, 1);
  Window::rename(StringView(name));
  return 0;
}
static int lWindowResized(lua_State *L) {
  lua_pushboolean(L, Window::resized());
  return 1;
}
static int lWindowRect(lua_State *L) {
  udNewOwned<Rect>(L, Window::rect());
  return 1;
}
static int lWindowTopLeft(lua_State *L) {
  udNewOwned<Vec2>(L, Window::topLeft());
  return 1;
}
static int lWindowTopCenter(lua_State *L) {
  udNewOwned<Vec2>(L, Window::topCenter());
  return 1;
}
static int lWindowTopRight(lua_State *L) {
  udNewOwned<Vec2>(L, Window::topRight());
  return 1;
}
static int lWindowLeft(lua_State *L) {
  udNewOwned<Vec2>(L, Window::left());
  return 1;
}
static int lWindowCenter(lua_State *L) {
  udNewOwned<Vec2>(L, Window::center());
  return 1;
}
static int lWindowRight(lua_State *L) {
  udNewOwned<Vec2>(L, Window::right());
  return 1;
}
static int lWindowBottomLeft(lua_State *L) {
  udNewOwned<Vec2>(L, Window::bottomLeft());
  return 1;
}
static int lWindowBottomCenter(lua_State *L) {
  udNewOwned<Vec2>(L, Window::bottomCenter());
  return 1;
}
static int lWindowBottomRight(lua_State *L) {
  udNewOwned<Vec2>(L, Window::bottomRight());
  return 1;
}
void registerWindow(lua_State *L) {
  pushSnNamed(L, "Window");
  Binding::registerFunction(L, "getName", lWindowGetName);
  Binding::registerFunction(L, "size", lWindowSize);
  Binding::registerFunction(L, "half", lWindowHalf);
  Binding::registerFunction(L, "resize", lWindowResize);
  Binding::registerFunction(L, "setFullscreen", lWindowSetFullscreen);
  Binding::registerFunction(L, "rename", lWindowRename);
  Binding::registerFunction(L, "resized", lWindowResized);
  Binding::registerFunction(L, "rect", lWindowRect);
  Binding::registerFunction(L, "topLeft", lWindowTopLeft);
  Binding::registerFunction(L, "topCenter", lWindowTopCenter);
  Binding::registerFunction(L, "topRight", lWindowTopRight);
  Binding::registerFunction(L, "left", lWindowLeft);
  Binding::registerFunction(L, "center", lWindowCenter);
  Binding::registerFunction(L, "right", lWindowRight);
  Binding::registerFunction(L, "bottomLeft", lWindowBottomLeft);
  Binding::registerFunction(L, "bottomCenter", lWindowBottomCenter);
  Binding::registerFunction(L, "bottomRight", lWindowBottomRight);
  lua_pop(L, 1);
}
} // namespace sinen
