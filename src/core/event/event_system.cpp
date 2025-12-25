#include "event_system.hpp"

namespace sinen {
void EventSystem::setQuit(bool quit) { EventSystem::quit = quit; }
bool EventSystem::isQuit() { return quit; }

void EventSystem::processEvent(SDL_Event &event) {
  switch (event.type) {
  case SDL_EVENT_QUIT: {
    setQuit(true);
  } break;
  default:
    break;
  }
}
} // namespace sinen