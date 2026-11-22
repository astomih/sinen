#include "luaapi.hpp"
#include <core/event/event.hpp>

namespace sinen {
static auto lEventQuit() {
  Event::quit();
  return;
}
void registerEvent(lua_State *L) {
  pushSnNamed(L, "Event");
  luau::table(L, -1).function("quit", lEventQuit);
  lua_pop(L, 1);
}
} // namespace sinen
