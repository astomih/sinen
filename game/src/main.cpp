#include "dungeon_generator.hpp"
#include <sinen/sinen.hpp>

#include <sol/sol.hpp>
int main(int argc, char *argv[]) {
  if (!sinen::initialize()) {
    return -1;
  }

  if (argc >= 2) {
#ifdef DEBUG
    sinen::scene::load(argv[1]);
#endif
  }
  auto &lua = *(sol::state *)sinen::script::get_state();

  auto table = lua.create_table("dts");
  table["dungeon_generator"] = dts::dungeon_generator;

  sinen::run();
  if (!sinen::shutdown()) {
    return -1;
  }
  return 0;
}
