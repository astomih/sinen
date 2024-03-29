#include "viewer.hpp"
#include <sinen/sinen.hpp>

int main(int argc, char *argv[]) {
  sinen::scene::change_implements<sinen::viewer>();
  sinen::scene::set_run_script(true);
  if (!sinen::initialize(argc, argv)) {
    return -1;
  }
  sinen::run();
  if (!sinen::shutdown()) {
    return -1;
  }
  return 0;
}
