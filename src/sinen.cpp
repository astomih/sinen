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
#include "asset/script/script_system.hpp"
#include "graphics/graphics_system.hpp"
#include "math/random_system.hpp"
#include "physics/physics_system.hpp"
#include "platform/input/input_system.hpp"
#include "platform/window/window_system.hpp"

#include "main_system.hpp"

// external
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>

namespace sinen {
bool Sinen::initialize(int argc, char *argv[]) {
  SDL_SetHint(SDL_HINT_APP_NAME, "Sinen");
  SDL_SetHint(SDL_HINT_ANDROID_TRAP_BACK_BUTTON, "1");
  SDL_Init(SDL_INIT_EVENTS);
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
    Logger::critical("Failed to initialize audio system");
    SoundSystem::shutdown();
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
  ScriptType scriptType = ScriptType::Lua;
  {
    File f;
    if (f.open(FileSystem::getAppBaseDirectory() + "/settings.json",
               File::Mode::r)) {

      void *buffer = calloc(f.size() + 10, 1);
      f.read(buffer, f.size(), 1);
      f.close();
      Json j;
      j.parse((char *)buffer);
      auto str = j["ScriptLanguage"].getString();
      if (str == "lua") {
        scriptType = ScriptType::Lua;
      } else if (str == "python") {
        scriptType = ScriptType::Python;
      } else {
        Logger::warn(
            "Unknown script language in settings.json, defaulting to Lua");
      }
    } else {
      f.close();
      if (!AssetIO::openAsString(AssetType::Script, "main.lua").empty()) {
        scriptType = ScriptType::Lua;
      } else if (!AssetIO::openAsString(AssetType::Script, "main.py").empty()) {
        scriptType = ScriptType::Python;
      }
    }
  }
  if (!ScriptSystem::Initialize(scriptType)) {
    Logger::critical("Failed to initialize script system");
    return false;
  }
  if (!RandomSystem::initialize()) {
    Logger::critical("Failed to initialize random system");
    return false;
  }
  MainSystem::initialize();
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
