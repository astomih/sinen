#include "editor.hpp"
#include <sinen/sinen.hpp>

int main(int argc, char *argv[]) {
  sinen::scene::change_implements<sinen::editor>();
  sinen::main::activate();
  return sinen::main::run();
}
