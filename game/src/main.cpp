#include "dungeon_generator.hpp"
#include <sinen/sinen.hpp>

#include <sol/sol.hpp>
int main(int argc, char *argv[]) {
  sinen::main::activate();
  auto &lua = *(sol::state *)sinen::script::get_state();
  auto table = lua.create_table("dts");
  table["dungeon_generator"] = dts::dungeon_generator;

  return sinen::main::run();
}
