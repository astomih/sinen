#include "window_native.hpp"
#include <platform/window/window.hpp>

#include <core/event/event.hpp>

#include <core/def/macro.hpp>
#include <core/logger/log.hpp>
#include <core/parser/json.hpp>
#include <platform/input/keyboard.hpp>
#include <platform/io/filesystem.hpp>

#ifdef SINEN_PLATFORM_EMSCRIPTEN
#include <emscripten/html5.h>
#endif

namespace sinen {
Vec2 windowSize = Vec2(1280.f, 720.f);
String windowName;
::SDL_Window *pSdlWindow = nullptr;
bool bResizedInFrame = false;
bool bFullscreen = false;

bool Window::initialize(StringView name) {
  windowName = name;
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

  pSdlWindow =
      SDL_CreateWindow(String(name).c_str(), static_cast<int>(windowSize.x),
                       static_cast<int>(windowSize.y), windowFlags);
  if (!pSdlWindow) {
    Log::error("SDL_CreateWindow failed: {}", SDL_GetError());
    return false;
  }

  int x = 0;
  int y = 0;
  SDL_GetWindowSize(pSdlWindow, &x, &y);
#ifdef SINEN_PLATFORM_EMSCRIPTEN
  const int fallbackW =
      static_cast<int>(windowSize.x > 0 ? windowSize.x : 1280.f);
  const int fallbackH =
      static_cast<int>(windowSize.y > 0 ? windowSize.y : 720.f);
  int canvasW = 0;
  int canvasH = 0;
  emscripten_get_canvas_element_size("#canvas", &canvasW, &canvasH);
  if (canvasW <= 0 || canvasH <= 0) {
    canvasW = fallbackW;
    canvasH = fallbackH;
    emscripten_set_canvas_element_size("#canvas", canvasW, canvasH);
    emscripten_set_element_css_size("#canvas", canvasW, canvasH);
    SDL_SetWindowSize(pSdlWindow, canvasW, canvasH);
  }
  if (canvasW > 0 && canvasH > 0) {
    x = canvasW;
    y = canvasH;
  }
#endif
  if (x > 0 && y > 0) {
    windowSize.x = static_cast<float>(x);
    windowSize.y = static_cast<float>(y);
  }
  return true;
}

bool Window::recreate() {
  const String name = windowName;
  if (pSdlWindow) {
    int width = 0;
    int height = 0;
    SDL_GetWindowSize(pSdlWindow, &width, &height);
    if (width > 0 && height > 0) {
      windowSize.x = static_cast<float>(width);
      windowSize.y = static_cast<float>(height);
    }
    SDL_DestroyWindow(pSdlWindow);
    pSdlWindow = nullptr;
  }

  if (!Window::initialize(name)) {
    return false;
  }
  if (bFullscreen) {
    SDL_SetWindowFullscreen(pSdlWindow, true);
  }
  bResizedInFrame = true;
  return true;
}

void Window::shutdown() {
  SDL_DestroyWindow(pSdlWindow);
  pSdlWindow = nullptr;
}
void Window::resize(const Vec2 &size) {
  windowSize = size;
  SDL_SetWindowSize(pSdlWindow, static_cast<int>(windowSize.x),
                    static_cast<int>(windowSize.y));
}
void Window::setFullscreen(bool fullscreen) {
  bFullscreen = fullscreen;
  SDL_SetWindowFullscreen(pSdlWindow, fullscreen);
}
void Window::rename(StringView name) {
  windowName = name;
  SDL_SetWindowTitle(pSdlWindow, windowName.c_str());
}

Vec2 Window::size() { return windowSize; }
bool Window::resized() { return bResizedInFrame; }
Vec2 Window::half() { return Vec2(windowSize.x / 2.0, windowSize.y / 2.0); }
String Window::name() { return windowName; }
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

void Window::prepareFrame() { bResizedInFrame = false; }
void Window::processEvent(const Event &event) {
  if (event.type() == Event::Type::WindowResized) {
    bResizedInFrame = true;
    int x, y;
    SDL_GetWindowSize(pSdlWindow, &x, &y);
    if (x > 0 && y > 0) {
      windowSize.x = static_cast<float>(x);
      windowSize.y = static_cast<float>(y);
    }
  }
}
SDL_Window *WindowNative::getWindow() { return pSdlWindow; }

} // namespace sinen
