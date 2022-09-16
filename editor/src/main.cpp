#include <sinen/sinen.hpp>

int main(int argc, char *argv[]) {
  if (!sinen::initialize())
    return -1;
  sinen::launch();
  return 0;
}
