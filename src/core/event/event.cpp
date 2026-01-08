#include "event.hpp"

namespace sinen {
void Event::setQuit(bool quit) { _quit = quit; }
bool Event::isQuit() { return _quit; }

void Event::processEvent(SDL_Event &event) {
  switch (event.type) {
  case SDL_EVENT_QUIT: {
    setQuit(true);
  } break;
  default:
    break;
  }
}
} // namespace sinen