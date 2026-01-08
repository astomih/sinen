#ifndef SINEN_WINDOW_SYSTEM_HPP
#define SINEN_WINDOW_SYSTEM_HPP
#include <core/data/string.hpp>
#include <graphics/graphics.hpp>
#include <math/vector.hpp>

#include <SDL3/SDL.h>
namespace sinen {
class Window {
public:
  static bool initialize(StringView name);
  static void shutdown();
  static void prepareFrame();
  static void processInput(SDL_Event &event);
  static SDL_Window *getSdlWindow() { return mWindow; }
  static void resize(const Vec2 &size);
  static void setFullscreen(bool fullscreen);
  static void rename(StringView name);

  static Vec2 size() { return mSize; }
  static bool resized() { return mResized; }
  static Vec2 half() { return Vec2(mSize.x / 2.0, mSize.y / 2.0); }
  static String name() { return mName; }

private:
  static bool mResized;
  static Vec2 mSize;
  static String mName;
  static ::SDL_Window *mWindow;
};
} // namespace sinen
#endif // SINEN_WINDOW_SYSTEM_HPP
