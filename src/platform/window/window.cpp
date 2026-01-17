#include "window.hpp"

#include <SDL3/SDL.h>
#include <core/parser/json.hpp>
#include <platform/input/keyboard.hpp>
#include <platform/io/file.hpp>
#include <platform/io/filesystem.hpp>

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
} // namespace sinen
