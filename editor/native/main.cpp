#include "custom_logger.hpp"
#include "editor.hpp"
#include <sinen/sinen.hpp>

int main(int argc, char *argv[]) {
  sinen::Scene::change_implements<sinen::editor>();
  sinen::Scene::set_run_script(true);
  sinen::custom_logger();
  if (!sinen::Initialize(argc, argv)) {
    return -1;
  }
  sinen::Window::rename("");
  sinen::Run();
  if (!sinen::Shutdown()) {
    return -1;
  }
  return 0;
}
