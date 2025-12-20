#ifndef SINEN_WINDOW_SYSTEM_HPP
#define SINEN_WINDOW_SYSTEM_HPP
#include "glm/ext/vector_float2.hpp"
#include <SDL3/SDL.h>
#include <graphics/graphics.hpp>
#include <memory>
#include <string>

namespace sinen {
class WindowSystem {
public:
  static bool initialize(const std::string &name);
  static void shutdown();
  static void prepareFrame();
  static void processInput(SDL_Event &event);
  static SDL_Window *getSdlWindow() { return mWindow; }
  static void resize(const glm::vec2 &size);
  static void setFullscreen(bool fullscreen);
  static void rename(const std::string &name);

  static glm::vec2 size() { return mSize; }
  static bool resized() { return mResized; }
  static glm::vec2 half() { return glm::vec2(mSize.x / 2.0, mSize.y / 2.0); }
  static std::string name() { return mName; }

private:
  static bool mResized;
  static glm::vec2 mSize;
  static std::string mName;
  static ::SDL_Window *mWindow;
};
} // namespace sinen
#endif // SINEN_WINDOW_SYSTEM_HPP
