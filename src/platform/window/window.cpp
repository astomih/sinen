#include "window.hpp"

#include <SDL3/SDL.h>
#include <core/io/file.hpp>
#include <core/io/file_system.hpp>
#include <core/io/json.hpp>
#include <platform/input/keyboard.hpp>

namespace sinen {
Vec2 Window::mSize = Vec2(1280.f, 720.f);
String Window::mName;
::SDL_Window *Window::mWindow = nullptr;
bool Window::mResized = false;
bool Window::initialize(StringView name) {
  mName = name;

  // Load settings from settings.json
  {
    File f;
    if (f.open(FileSystem::getAppBaseDirectory() + "/settings.json", "r")) {
      auto size = f.size();
      auto buffer = f.read(size + 1);
      f.close();
      Json j;
      j.parse((char *)buffer.data());
      mSize.x = j["WindowWidth"].getFloat();
      mSize.y = j["WindowHeight"].getFloat();
    } else {
      f.close();
      mSize.x = 1280;
      mSize.y = 720;
    }
  }

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
void Window::prepareFrame() { mResized = false; }
void Window::processInput(SDL_Event &event) {
  int x, y;
  SDL_GetWindowSize(mWindow, &x, &y);
  mSize.x = static_cast<float>(x);
  mSize.y = static_cast<float>(y);
  if (event.window.type == SDL_EventType::SDL_EVENT_WINDOW_RESIZED) {
    mResized = true;
  }
  if (Keyboard::isPressed(Keyboard::Code::F11)) {
    static bool fullscreen = false;
    fullscreen = !fullscreen;
    setFullscreen(fullscreen);
  }
}
} // namespace sinen
