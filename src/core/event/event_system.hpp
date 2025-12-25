#ifndef SINEN_EVENT_SYSTEM_HPP
#define SINEN_EVENT_SYSTEM_HPP
#include <SDL3/SDL.h>

namespace sinen {
class EventSystem {
public:
  static void setQuit(bool quit);
  static bool isQuit();

  static void processEvent(SDL_Event &event);

private:
  inline static bool quit = false;
};
} // namespace sinen

#endif