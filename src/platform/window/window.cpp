#include "window_system.hpp"
#include <SDL3/SDL.h>
#include <core/io/file.hpp>
#include <core/io/file_system.hpp>
#include <core/io/json.hpp>
#include <platform/input/keyboard.hpp>
#include <platform/window/window.hpp>

namespace sinen {
glm::vec2 WindowSystem::mSize = glm::vec2(1280.f, 720.f);
std::string WindowSystem::mName = "";
::SDL_Window *WindowSystem::mWindow = nullptr;
bool WindowSystem::mResized = false;
const void *Window::getSDLWindow() { return WindowSystem::getSdlWindow(); }
glm::vec2 Window::size() { return WindowSystem::size(); }
glm::vec2 Window::half() { return WindowSystem::half(); }
void Window::resize(const glm::vec2 &size) { WindowSystem::resize(size); }
void Window::setFullscreen(bool fullscreen) {
  WindowSystem::setFullscreen(fullscreen);
}
void Window::rename(const std::string &name) { WindowSystem::rename(name); }
std::string Window::getName() { return WindowSystem::name(); }
bool Window::resized() { return WindowSystem::resized(); }

void WindowSystem::initialize(const std::string &name) {
  mName = name;

  // Load settings from settings.json
  {
    File f;
    if (f.open(FileSystem::getAppBaseDirectory() + "/settings.json",
               File::Mode::r)) {
      void *buffer = calloc(f.size() + 10, 1);
      f.read(buffer, f.size(), 1);
      f.close();
      Json j;
      j.parse((char *)buffer);
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

  mWindow =
      SDL_CreateWindow(std::string(name).c_str(), static_cast<int>(mSize.x),
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
}

void WindowSystem::shutdown() {
  SDL_DestroyWindow(mWindow);
  mWindow = nullptr;
}
void WindowSystem::resize(const glm::vec2 &size) {
  mSize = size;
  SDL_SetWindowSize(mWindow, static_cast<int>(mSize.x),
                    static_cast<int>(mSize.y));
}
void WindowSystem::setFullscreen(bool fullscreen) {
  SDL_SetWindowFullscreen(mWindow, fullscreen);
}
void WindowSystem::rename(const std::string &name) {
  mName = name;
  SDL_SetWindowTitle(mWindow, mName.c_str());
}
void WindowSystem::prepareFrame() { mResized = false; }
void WindowSystem::processInput(SDL_Event &event) {
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
