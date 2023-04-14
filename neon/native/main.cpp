#include <sinen/sinen.hpp>

int main(int argc, char *argv[]) {
  sinen::scene::set_run_script(false);
  if (!sinen::initialize(argc, argv)) {
    return -1;
  }
  sinen::window::rename("neon");
  sinen::run();
  if (!sinen::shutdown()) {
    return -1;
  }
  return 0;
}