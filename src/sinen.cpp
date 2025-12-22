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
  if (!MainSystem::initialize(argc, argv)) {
    Logger::critical("Failed to initialize main system");
    return false;
  }
  if (!WindowSystem::initialize("Sinen")) {
    Logger::critical("Failed to initialize window system");
    return false;
  }
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
  if (!PhysicsSystem::initialize()) {
    Logger::critical("Failed to initialize physics system");
    return false;
  }
  if (!ScriptSystem::initialize(ScriptType::Lua)) {
    Logger::critical("Failed to initialize script system");
    return false;
  }
  if (!RandomSystem::initialize()) {
    Logger::critical("Failed to initialize random system");
    return false;
  }
  return true;
}
void Sinen::run() {
  while (true) {
    if (MainSystem::isRunning()) {
      WindowSystem::prepareFrame();
      InputSystem::prepareForUpdate();
      MainSystem::processInput();
      InputSystem::update();
      MainSystem::updateScene();
      GraphicsSystem::render();
      continue;
    }
    if (MainSystem::isReset) {
      MainSystem::setup();
      MainSystem::isReset = false;
      continue;
    }
    break;
  }
}
void Sinen::shutdown() {
  MainSystem::shutdown();
  PhysicsSystem::shutdown();
  InputSystem::shutdown();
  AudioSystem::shutdown();
  RandomSystem::shutdown();
  GraphicsSystem::shutdown();
  WindowSystem::shutdown();
  ScriptSystem::shutdown();
  SDL_Quit();
}
} // namespace sinen
