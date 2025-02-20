// std

// internal
#include <sinen.hpp>

#include "audio/sound_system.hpp"
#include "input/input_system.hpp"
#include "math/random_system.hpp"
#include "render/render_system.hpp"
#include "scene/scene_system.hpp"
#include "script/script_system.hpp"
#include "window/window_system.hpp"

// external
#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>

namespace sinen {
bool Initialize(int argc, char *argv[]) {
  SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK |
           SDL_INIT_HAPTIC | SDL_INIT_GAMEPAD | SDL_INIT_SENSOR);
  TTF_Init();
  Mix_Init(MIX_INIT_OGG);
  SDL_AudioSpec desired;
  desired.freq = 44100;
  desired.format = MIX_DEFAULT_FORMAT;
  desired.channels = 2;
  Mix_OpenAudio(44100, &desired);
  WindowImpl::initialize("SinenEngine");
  RendererImpl::initialize();
  if (!sound_system::initialize()) {
    Logger::critical("Failed to initialize audio system");
    sound_system::shutdown();
    return false;
  }
  if (!input_system::initialize()) {
    Logger::critical("Failed to initialize input system");
    return false;
  }
  if (!script_system::initialize()) {
    Logger::critical("Failed to initialize script system");
    return false;
  }
  if (!random_system::initialize()) {
    Logger::critical("Failed to initialize random system");
    return false;
  }
  Texture tex;
  tex.fill_color(Palette::light_black());
  scene_system::initialize();
  return true;
}
void Run() {
  while (true) {
    if (scene_system::is_running()) {
      WindowImpl::prepare_frame();
      scene_system::process_input();
      scene_system::update_scene();
      input_system::prepare_for_update();
      input_system::update();
      RendererImpl::render();
      continue;
    }
    if (scene_system::is_reset) {
      scene_system::setup();
      scene_system::is_reset = false;
      continue;
    }
    break;
  }
}
bool Shutdown() {
  scene_system::shutdown();
  script_system::shutdown();
  input_system::shutdown();
  sound_system::shutdown();
  random_system::shutdown();
  RendererImpl::shutdown();
  WindowImpl::shutdown();
  Mix_CloseAudio();
  TTF_Quit();
  Mix_Quit();
  SDL_Quit();
  return true;
}
} // namespace sinen
