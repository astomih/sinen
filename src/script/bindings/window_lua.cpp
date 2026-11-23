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
void registerWindow(luau::table &sn) {
  auto window = sn.get_or_create_table("Window");
  window.function<lWindowGetName>("getName");
  window.function<lWindowSize>("size");
  window.function<lWindowHalf>("half");
  window.function<lWindowResize>("resize");
  window.raw_function<lWindowSetFullscreen>("setFullscreen");
  window.function<lWindowRename>("rename");
  window.function<lWindowResized>("resized");
  window.function<lWindowHasFocus>("hasFocus");
  window.function<lWindowRect>("rect");
  window.function<lWindowTopLeft>("topLeft");
  window.function<lWindowTopCenter>("topCenter");
  window.function<lWindowTopRight>("topRight");
  window.function<lWindowLeft>("left");
  window.function<lWindowCenter>("center");
  window.function<lWindowRight>("right");
  window.function<lWindowBottomLeft>("bottomLeft");
  window.function<lWindowBottomCenter>("bottomCenter");
  window.function<lWindowBottomRight>("bottomRight");
}
} // namespace sinen
