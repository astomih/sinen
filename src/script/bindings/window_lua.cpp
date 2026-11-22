#include "luaapi.hpp"
#include <core/def/macro.hpp>
#include <core/parser/json.hpp>
#include <platform/input/keyboard.hpp>
#include <platform/io/filesystem.hpp>
#include <platform/window/window.hpp>

namespace sinen {
static auto lWindowGetName() {
  auto name = Window::name();
  return name;
}
static auto lWindowSize() { return Window::size(); }
static auto lWindowHalf() { return Window::half(); }
static auto lWindowResize(Vec2 &size) {
  Window::resize(size);
  return;
}
static int lWindowSetFullscreen(lua_State *L) {
  bool fs = lua_toboolean(L, 1) != 0;
  Window::setFullscreen(fs);
  return 0;
}
static auto lWindowRename(const char *name) {
  Window::rename(StringView(name));
  return;
}
static auto lWindowResized() { return Window::resized(); }
static auto lWindowHasFocus() { return Window::hasFocus(); }
static auto lWindowRect() { return Window::rect(); }
static auto lWindowTopLeft() { return Window::topLeft(); }
static auto lWindowTopCenter() { return Window::topCenter(); }
static auto lWindowTopRight() { return Window::topRight(); }
static auto lWindowLeft() { return Window::left(); }
static auto lWindowCenter() { return Window::center(); }
static auto lWindowRight() { return Window::right(); }
static auto lWindowBottomLeft() { return Window::bottomLeft(); }
static auto lWindowBottomCenter() { return Window::bottomCenter(); }
static auto lWindowBottomRight() { return Window::bottomRight(); }
void registerWindow(lua_State *L) {
  pushSnNamed(L, "Window");
  luau::table(L, -1).function("getName", lWindowGetName);
  luau::table(L, -1).function("size", lWindowSize);
  luau::table(L, -1).function("half", lWindowHalf);
  luau::table(L, -1).function("resize", lWindowResize);
  luau::table(L, -1).raw_function("setFullscreen", lWindowSetFullscreen);
  luau::table(L, -1).function("rename", lWindowRename);
  luau::table(L, -1).function("resized", lWindowResized);
  luau::table(L, -1).function("hasFocus", lWindowHasFocus);
  luau::table(L, -1).function("rect", lWindowRect);
  luau::table(L, -1).function("topLeft", lWindowTopLeft);
  luau::table(L, -1).function("topCenter", lWindowTopCenter);
  luau::table(L, -1).function("topRight", lWindowTopRight);
  luau::table(L, -1).function("left", lWindowLeft);
  luau::table(L, -1).function("center", lWindowCenter);
  luau::table(L, -1).function("right", lWindowRight);
  luau::table(L, -1).function("bottomLeft", lWindowBottomLeft);
  luau::table(L, -1).function("bottomCenter", lWindowBottomCenter);
  luau::table(L, -1).function("bottomRight", lWindowBottomRight);
  lua_pop(L, 1);
}
} // namespace sinen
