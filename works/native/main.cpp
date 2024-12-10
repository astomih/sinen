#include "dungeon_generator.hpp"
#include <sinen/sinen.hpp>

#include <sol/sol.hpp>
int main(int argc, char *argv[]) {
  if (!sinen::Initialize(argc, argv)) {
    return -1;
  }
  auto &lua = *(sol::state *)sinen::Script::get_state();
  auto table = lua.create_table("dts");
  table["dungeon_generator"] = dts::dungeon_generator;
  sinen::Run();
  if (!sinen::Shutdown()) {
    return -1;
  }
  return 0;
}
