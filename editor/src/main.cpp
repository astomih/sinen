#include "custom_logger.hpp"
#include "editor.hpp"
#include <sinen/sinen.hpp>

int main(int argc, char *argv[]) {
  sinen::scene::change_implements<sinen::editor>();
  sinen::scene::set_run_script(false);
  sinen::logger::change_logger(std::make_unique<sinen::imgui_logger>());
  sinen::main::activate();
  return sinen::main::run();
}
