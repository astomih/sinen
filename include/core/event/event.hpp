#ifndef SINEN_EVENT_SYSTEM_HPP
#define SINEN_EVENT_SYSTEM_HPP
#include <SDL3/SDL.h>

namespace sinen {
class Event {
public:
  static void quit() { setQuit(true); }
  static void setQuit(bool quit);
  static bool isQuit();
  static bool isPaused();

  static void processEvent(SDL_Event &event);

private:
  static bool _quit;
  static bool paused;
};
} // namespace sinen

#endif