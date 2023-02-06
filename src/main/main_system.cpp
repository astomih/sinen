#ifndef MOBILE
#define SDL_MAIN_HANDLED
#else
#include <SDL_main.h>
#endif

#include "main_system.hpp"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_net.h>
#include <SDL_ttf.h>
#include <fstream>
#include <memory>
#include <optional>

#include <camera/camera.hpp>
#include <input/input.hpp>
#include <io/file.hpp>
#include <io/json.hpp>
#include <logger/logger.hpp>
#include <main/main.hpp>
#include <math/random.hpp>
#include <render/renderer.hpp>
#include <script/script.hpp>
#include <window/window.hpp>

#include "../audio/sound_system.hpp"
#include "../font/font_system.hpp"
#include "../input/input_system.hpp"
#include "../math/random_system.hpp"
#include "../render/render_system.hpp"
#include "../scene/scene_system.hpp"
#include "../script/script_system.hpp"
#include "../texture/texture_system.hpp"
#include "../window/window_system.hpp"
#ifdef EMSCRIPTEN
#include <emscripten.h>

std::function<void()> emscripten_loop;
void main_loop() { emscripten_loop(); }
#endif

namespace sinen {
bool main_system::is_reset = true;
std::string main_system::m_scene_name = "main";
int main::run() {
  main_system::run();
  if (!deactivate()) {
    return -1;
  }
  logger::info("Quit Sinen Program");
  return 0;
}
bool main::activate() {
  if (!main_system::initialize()) {
    return false;
  }
  logger::info("Complete activate");
  return true;
}
bool main::deactivate() {
  logger::info("Sub system deactivating");
  scene_system::shutdown();
  script_system::shutdown();
  input_system::shutdown();
  sound_system::shutdown();
  font_system::shutdown();
  random_system::shutdown();
  texture_system::shutdown();
  render_system::shutdown();
  window_system::shutdown();
  logger::info("Main system deactivating");
  Mix_CloseAudio();
  TTF_Quit();
  SDLNet_Quit();
  Mix_Quit();
  IMG_Quit();
  SDL_Quit();
  logger::info("Complete deactivate");
  return true;
}
void main::change_scene(const std::string &scene_number) {
  main_system::change_scene<scene>(scene_number);
}
std::string main::get_current_scene_number() {
  return main_system::get_current_scene_number();
}
bool main_system::initialize() {
  logger::info("Main system activating");
  SDL_SetMainReady();
  SDL_Init(SDL_INIT_EVERYTHING);
  TTF_Init();
  IMG_Init(IMG_INIT_PNG);
  SDLNet_Init();
  Mix_Init(MIX_INIT_OGG);
  Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
  logger::info("Sub system activating");
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
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
  if (str.compare("Vulkan") == 0) {
    window_system::initialize("SinenEngine(Vulkan)", graphics_api::Vulkan);
    render_system::initialize(graphics_api::Vulkan);
  } else if (str.compare("OpenGL") == 0) {
    window_system::initialize("SinenEngine(OpenGL)", graphics_api::OpenGL);
    render_system::initialize(graphics_api::OpenGL);
  }

#else
  window_system::initialize("SinenEngine", graphics_api::ES);
  render_system::initialize(graphics_api::ES);
#endif
  if (!sound_system::initialize()) {
    logger::fatal("Failed to initialize audio system");
    sound_system::shutdown();
    return false;
  }
  if (!input_system::initialize()) {
    logger::fatal("Failed to initialize input system");
    return false;
  }
  if (!script_system::initialize()) {
    logger::fatal("Failed to initialize script system");
    return false;
  }
  if (!font_system::initialize()) {
    logger::fatal("Failed to initialize font system");
    return false;
  }
  if (!random_system::initialize()) {
    logger::fatal("Failed to initialize random system");
    return false;
  }
  texture tex;
  tex.fill_color(palette::light_black());
  render_system::set_skybox_texture(tex);
  scene_system::initialize();
  return true;
}
void main_system::run() {

#if !defined(EMSCRIPTEN)
  while (loop()) {
  }
#else
  emscripten_loop = [&]() { loop(); };
  emscripten_set_main_loop(main_loop, 120, true);
#endif
}
bool main_system::loop() {
  if (scene_system::is_running()) {
    scene_system::run_loop();
    return true;
  }
  if (is_reset) {
    scene_system::setup();
    is_reset = false;
    return true;
  }
  return false;
}
void main_system::change_scene_impl(const std::string &scene_name) {
  if (scene_name.empty()) {
    scene::set_state(scene::state::quit);
    is_reset = false;
  } else {
    is_reset = true;
  }

  scene_system::shutdown();
  script_system::shutdown();
  m_scene_name = scene_name;
}
} // namespace sinen
