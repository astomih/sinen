#include <sinen/sinen.hpp>

#include <SDL3/SDL_main.h>

int main(int argc, char *argv[]) {
  if (!sinen::Initialize(argc, argv)) {
    return -1;
  }
  sinen::Run();
  if (!sinen::Shutdown()) {
    return -1;
  }
  return 0;
}
