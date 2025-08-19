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

#include "asset/audio/sound_system.hpp"
#include "graphics/graphics_system.hpp"
#include "logic/scene/scene_system.hpp"
#include "logic/script/script_system.hpp"
#include "math/random_system.hpp"
#include "platform/input/input_system.hpp"
#include "platform/window/window_system.hpp"

// external
#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>

namespace sinen {
bool Sinen::initialize(int argc, char *argv[]) {
  SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK |
           SDL_INIT_HAPTIC | SDL_INIT_GAMEPAD | SDL_INIT_SENSOR);
  TTF_Init();
  Mix_Init(MIX_INIT_OGG);
  SDL_AudioSpec desired;
  desired.freq = 48000;
  desired.format = MIX_DEFAULT_FORMAT;
  desired.channels = 2;
  auto devid = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &desired);
  Mix_OpenAudio(devid, &desired);
  WindowSystem::initialize("Sinen");
  GraphicsSystem::initialize();
  if (!SoundSystem::initialize()) {
    Logger::Critical("Failed to initialize audio system");
    SoundSystem::shutdown();
    return false;
  }
  if (!InputSystem::initialize()) {
    Logger::Critical("Failed to initialize input system");
    return false;
  }
  if (!ScriptSystem::Initialize(ScriptType::Python)) {
    Logger::Critical("Failed to initialize script system");
    return false;
  }
  if (!RandomSystem::initialize()) {
    Logger::Critical("Failed to initialize random system");
    return false;
  }
  SceneSystem::initialize();
  return true;
}
void Sinen::run() {
  while (true) {
    if (SceneSystem::is_running()) {
      WindowSystem::prepare_frame();
      InputSystem::prepare_for_update();
      SceneSystem::process_input();
      InputSystem::update();
      SceneSystem::update_scene();
      GraphicsSystem::render();
      continue;
    }
    if (SceneSystem::is_reset) {
      SceneSystem::setup();
      SceneSystem::is_reset = false;
      continue;
    }
    break;
  }
}
void Sinen::shutdown() {
  SceneSystem::shutdown();
  ScriptSystem::Shutdown();
  InputSystem::shutdown();
  SoundSystem::shutdown();
  RandomSystem::shutdown();
  GraphicsSystem::shutdown();
  WindowSystem::shutdown();
  Mix_CloseAudio();
  TTF_Quit();
  Mix_Quit();
  SDL_Quit();
}
} // namespace sinen
