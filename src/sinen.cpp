// internal
#include <SDL3/SDL_main.h>
#include <sinen.hpp>
int main(const int argc, char *argv[]) {
  using namespace sinen;
  if (!Sinen::initialize(argc, argv)) {
    return -1;
  }
  Sinen::run();
  Sinen::shutdown();
  return 0;
}

#include "asset/audio/audio_system.hpp"
#include "asset/script/script_system.hpp"
#include "graphics/graphics_system.hpp"
#include "math/random_system.hpp"
#include "physics/physics_system.hpp"
#include "platform/input/input_system.hpp"
#include "platform/window/window_system.hpp"

#include "main_system.hpp"

// external
#include <SDL3/SDL.h>

namespace sinen {
bool Sinen::initialize(int argc, char *argv[]) {
  SDL_SetHint(SDL_HINT_APP_NAME, "Sinen");
  SDL_SetHint(SDL_HINT_ANDROID_TRAP_BACK_BUTTON, "1");
  SDL_Init(SDL_INIT_EVENTS);
  WindowSystem::initialize("Sinen");
  GraphicsSystem::initialize();
  if (!AudioSystem::initialize()) {
    Logger::critical("Failed to initialize audio system");
    AudioSystem::shutdown();
    return false;
  }
  if (!InputSystem::initialize()) {
    Logger::critical("Failed to initialize input system");
    return false;
  }
  if (!PhysicsSystem::Initialize()) {
    Logger::critical("Failed to initialize physics system");
    return false;
  }
  if (!ScriptSystem::Initialize(ScriptType::Lua)) {
    Logger::critical("Failed to initialize script system");
    return false;
  }
  if (!RandomSystem::initialize()) {
    Logger::critical("Failed to initialize random system");
    return false;
  }
  MainSystem::initialize(argc, argv);
  return true;
}
void Sinen::run() {
  while (true) {
    if (MainSystem::is_running()) {
      WindowSystem::prepare_frame();
      InputSystem::prepare_for_update();
      MainSystem::process_input();
      InputSystem::update();
      MainSystem::update_scene();
      GraphicsSystem::render();
      continue;
    }
    if (MainSystem::is_reset) {
      MainSystem::setup();
      MainSystem::is_reset = false;
      continue;
    }
    break;
  }
}
void Sinen::shutdown() {
  MainSystem::shutdown();
  PhysicsSystem::Shutdown();
  ScriptSystem::Shutdown();
  InputSystem::shutdown();
  AudioSystem::shutdown();
  RandomSystem::shutdown();
  GraphicsSystem::shutdown();
  WindowSystem::shutdown();
  SDL_Quit();
}
} // namespace sinen
