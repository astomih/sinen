#include "event.hpp"
#include <script/luaapi.hpp>

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

static int lEventQuit(lua_State *L) {
  (void)L;
  Event::quit();
  return 0;
}
void registerEvent(lua_State *L) {
  pushSnNamed(L, "Event");
  luaPushcfunction2(L, lEventQuit);
  lua_setfield(L, -2, "quit");
  lua_pop(L, 1);
}
} // namespace sinen
