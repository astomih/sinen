// std
#include <fstream>
#include <memory>
#include <optional>

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
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_net.h>
#include <SDL_ttf.h>

namespace sinen {
bool initialize() {
  SDL_SetMainReady();
  SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_EVENTS |
           SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_GAMECONTROLLER |
           SDL_INIT_SENSOR);
  TTF_Init();
  IMG_Init(IMG_INIT_PNG);
  SDLNet_Init();
  Mix_Init(MIX_INIT_OGG);
  Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
  std::string str;
  file f;
  if (f.open("settings.json", file::mode::r)) {
    char raw[256] = {};
    f.read(raw, 256, 1);
    auto s = std::string(raw);
    json j;
    j.parse(s);
    str = j["GraphicsAPI"].get_string();
    f.close();

  } else
    str = "Vulkan";
  window_system::initialize("SinenEngine", graphics_api::Vulkan);
  render_system::initialize(graphics_api::Vulkan);

  if (!sound_system::initialize()) {
    logger::critical("Failed to initialize audio system");
    sound_system::shutdown();
    return false;
  }
  if (!input_system::initialize()) {
    logger::critical("Failed to initialize input system");
    return false;
  }
  if (!script_system::initialize()) {
    logger::critical("Failed to initialize script system");
    return false;
  }
  if (!random_system::initialize()) {
    logger::critical("Failed to initialize random system");
    return false;
  }
  texture tex;
  tex.fill_color(palette::light_black());
  render_system::set_skybox_texture(tex);
  scene_system::initialize();
  return true;
}
void run() {
  while (scene_system::run_loop()) {
  }
}
bool shutdown() {
  scene_system::shutdown();
  script_system::shutdown();
  input_system::shutdown();
  sound_system::shutdown();
  random_system::shutdown();
  render_system::shutdown();
  window_system::shutdown();
  Mix_CloseAudio();
  TTF_Quit();
  SDLNet_Quit();
  Mix_Quit();
  IMG_Quit();
  SDL_Quit();
  return true;
}
} // namespace sinen
