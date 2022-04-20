#include <Nen/Nen.hpp>

int main(int argc, char *argv[]) {
  if (!nen::initialize())
    return -1;
  nen::launch();
  return 0;
}