#include "window.hpp"

#include <SDL3/SDL.h>
#include <core/parser/json.hpp>
#include <platform/input/keyboard.hpp>
#include <platform/io/file.hpp>
#include <platform/io/filesystem.hpp>
#include <script/luaapi.hpp>

namespace sinen {
Vec2 Window::mSize = Vec2(1280.f, 720.f);
String Window::mName;
::SDL_Window *Window::mWindow = nullptr;
bool Window::mResized = false;
bool Window::initialize(StringView name) {
  mName = name;
  uint64_t windowFlags = SDL_WINDOW_VULKAN;
#ifdef __ANDROID__
  windowFlags |=
      SDL_WINDOW_FULLSCREEN | SDL_WINDOW_BORDERLESS | SDL_WINDOW_INPUT_FOCUS;

#else
  windowFlags |= SDL_WINDOW_RESIZABLE;
#endif

  mWindow = SDL_CreateWindow(String(name).c_str(), static_cast<int>(mSize.x),
                             static_cast<int>(mSize.y), windowFlags);

  // Safe rect
  // #ifdef __ANDROID__
  //   SDL_Rect safeArea;
  //   SDL_GetWindowSafeArea(mWindow, &safeArea);
  //   mSize.x = static_cast<float>(safeArea.w);
  //   mSize.y = static_cast<float>(safeArea.h);
  // #endif
  int x, y;
  SDL_GetWindowSize(mWindow, &x, &y);
  mSize.x = static_cast<float>(x);
  mSize.y = static_cast<float>(y);
  return true;
}

void Window::shutdown() {
  SDL_DestroyWindow(mWindow);
  mWindow = nullptr;
}
void Window::resize(const Vec2 &size) {
  mSize = size;
  SDL_SetWindowSize(mWindow, static_cast<int>(mSize.x),
                    static_cast<int>(mSize.y));
}
void Window::setFullscreen(bool fullscreen) {
  SDL_SetWindowFullscreen(mWindow, fullscreen);
}
void Window::rename(StringView name) {
  mName = name;
  SDL_SetWindowTitle(mWindow, mName.c_str());
}

Vec2 Window::size() { return mSize; }
bool Window::resized() { return mResized; }
String Window::name() { return mName; }
Rect Window::rect() { return Rect(Vec2(0), size()); }
Vec2 Window::topLeft() { return rect().topLeft(); }
Vec2 Window::topCenter() { return rect().topCenter(); }
Vec2 Window::topRight() { return rect().topRight(); }
Vec2 Window::left() { return rect().left(); }
Vec2 Window::center() { return rect().center(); }
Vec2 Window::right() { return rect().right(); }
Vec2 Window::bottomLeft() { return rect().bottomLeft(); }
Vec2 Window::bottomCenter() { return rect().bottomCenter(); }
Vec2 Window::bottomRight() { return rect().bottomRight(); }

void Window::prepareFrame() { mResized = false; }
void Window::processEvent(SDL_Event &event) {
  if (event.window.type == SDL_EventType::SDL_EVENT_WINDOW_RESIZED) {
    mResized = true;
    int x, y;
    SDL_GetWindowSize(mWindow, &x, &y);
    mSize.x = static_cast<float>(x);
    mSize.y = static_cast<float>(y);
  }
}

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
  udNewOwned<Vec2>(L, Window::left());
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
  luaPushcfunction2(L, lWindowGetName);
  lua_setfield(L, -2, "getName");
  luaPushcfunction2(L, lWindowSize);
  lua_setfield(L, -2, "size");
  luaPushcfunction2(L, lWindowHalf);
  lua_setfield(L, -2, "half");
  luaPushcfunction2(L, lWindowResize);
  lua_setfield(L, -2, "resize");
  luaPushcfunction2(L, lWindowSetFullscreen);
  lua_setfield(L, -2, "setFullscreen");
  luaPushcfunction2(L, lWindowRename);
  lua_setfield(L, -2, "rename");
  luaPushcfunction2(L, lWindowResized);
  lua_setfield(L, -2, "resized");
  luaPushcfunction2(L, lWindowRect);
  lua_setfield(L, -2, "rect");
  luaPushcfunction2(L, lWindowTopLeft);
  lua_setfield(L, -2, "topLeft");
  luaPushcfunction2(L, lWindowTopCenter);
  lua_setfield(L, -2, "topCenter");
  luaPushcfunction2(L, lWindowTopRight);
  lua_setfield(L, -2, "topRight");
  luaPushcfunction2(L, lWindowLeft);
  lua_setfield(L, -2, "left");
  luaPushcfunction2(L, lWindowCenter);
  lua_setfield(L, -2, "center");
  luaPushcfunction2(L, lWindowRight);
  lua_setfield(L, -2, "right");
  luaPushcfunction2(L, lWindowBottomLeft);
  lua_setfield(L, -2, "bottomLeft");
  luaPushcfunction2(L, lWindowBottomCenter);
  lua_setfield(L, -2, "bottomCenter");
  luaPushcfunction2(L, lWindowBottomRight);
  lua_setfield(L, -2, "bottomRight");
  lua_pop(L, 1);
}
} // namespace sinen
