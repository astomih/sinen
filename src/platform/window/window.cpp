#include "window.hpp"

#include <SDL3/SDL.h>
#include <core/def/macro.hpp>
#include <core/parser/json.hpp>
#include <platform/input/keyboard.hpp>
#include <platform/io/filesystem.hpp>

#ifdef SINEN_PLATFORM_EMSCRIPTEN
#include <emscripten/html5.h>
#endif

namespace sinen {
Vec2 Window::mSize = Vec2(1280.f, 720.f);
String Window::mName;
::SDL_Window *Window::mWindow = nullptr;
bool Window::mResized = false;
bool Window::initialize(StringView name) {
  mName = name;
  uint64_t windowFlags = 0;
#if !defined(SINEN_PLATFORM_EMSCRIPTEN)
  windowFlags |= SDL_WINDOW_VULKAN;
#endif
#ifdef __ANDROID__
  windowFlags |=
      SDL_WINDOW_FULLSCREEN | SDL_WINDOW_BORDERLESS | SDL_WINDOW_INPUT_FOCUS;

#else
  windowFlags |= SDL_WINDOW_RESIZABLE;
#endif

  mWindow = SDL_CreateWindow(String(name).c_str(), static_cast<int>(mSize.x),
                             static_cast<int>(mSize.y), windowFlags);
  if (!mWindow) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateWindow failed: %s",
                 SDL_GetError());
    return false;
  }

  // Safe rect
  // #ifdef __ANDROID__
  //   SDL_Rect safeArea;
  //   SDL_GetWindowSafeArea(mWindow, &safeArea);
  //   mSize.x = static_cast<float>(safeArea.w);
  //   mSize.y = static_cast<float>(safeArea.h);
  // #endif
  int x = 0;
  int y = 0;
  SDL_GetWindowSize(mWindow, &x, &y);
#ifdef SINEN_PLATFORM_EMSCRIPTEN
  const int fallbackW = static_cast<int>(mSize.x > 0 ? mSize.x : 1280.f);
  const int fallbackH = static_cast<int>(mSize.y > 0 ? mSize.y : 720.f);
  int canvasW = 0;
  int canvasH = 0;
  emscripten_get_canvas_element_size("#canvas", &canvasW, &canvasH);
  if (canvasW <= 0 || canvasH <= 0) {
    canvasW = fallbackW;
    canvasH = fallbackH;
    emscripten_set_canvas_element_size("#canvas", canvasW, canvasH);
    emscripten_set_element_css_size("#canvas", canvasW, canvasH);
    SDL_SetWindowSize(mWindow, canvasW, canvasH);
  }
  if (canvasW > 0 && canvasH > 0) {
    x = canvasW;
    y = canvasH;
  }
#endif
  if (x > 0 && y > 0) {
    mSize.x = static_cast<float>(x);
    mSize.y = static_cast<float>(y);
  }
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
    if (x > 0 && y > 0) {
      mSize.x = static_cast<float>(x);
      mSize.y = static_cast<float>(y);
    }
  }
}

} // namespace sinen
