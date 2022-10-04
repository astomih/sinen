#include <sinen/sinen.hpp>

int main(int argc, char *argv[]) {
  if (!sinen::main::initialize())
    return -1;
  sinen::main::launch();
  return 0;
}
