#include "luaapi.hpp"
#include <core/event/event.hpp>

namespace sinen {
static int lEventQuit(lua_State *L) {
  (void)L;
  Event::quit();
  return 0;
}
void registerEvent(lua_State *L) {
  pushSnNamed(L, "Event");
  Binding::registerFunction(L, "quit", lEventQuit);
  lua_pop(L, 1);
}
} // namespace sinen
