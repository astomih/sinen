#include "luaapi.hpp"
#include <core/event/event.hpp>

namespace sinen {
static auto lEventQuit() {
  Event::quit();
  return;
}
void registerEvent(luau::table &sn) {
  auto event = sn.get_or_create_table("Event");
  event.function<lEventQuit>("quit");
}
} // namespace sinen
