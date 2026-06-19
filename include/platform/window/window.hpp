#ifndef SINEN_WINDOW_SYSTEM_HPP
#define SINEN_WINDOW_SYSTEM_HPP
#include <core/data/string.hpp>
#include <graphics/graphics.hpp>
#include <math/vector.hpp>

#include <SDL3/SDL_video.h>

namespace sinen {
class Event;

class Window {
public:
  static bool initialize(StringView name);
  static bool recreate();
  static void shutdown();
  static void prepareFrame();
  static void processEvent(const Event &event);
  static SDL_Window *getSdlWindow();
  static void resize(const Vec2 &size);
  static void setFullscreen(bool fullscreen);
  static void rename(StringView name);

  static Vec2 size();
  static bool resized();
  static Vec2 half();
  static Rect rect();
  static Vec2 topLeft();
  static Vec2 topCenter();
  static Vec2 topRight();
  static Vec2 left();
  static Vec2 center();
  static Vec2 right();
  static Vec2 bottomLeft();
  static Vec2 bottomCenter();
  static Vec2 bottomRight();
  static String name();
};
} // namespace sinen
#endif // SINEN_WINDOW_SYSTEM_HPP
