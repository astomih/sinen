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
#include <logger/logger.hpp>
#include <main/main.hpp>
#include <math/random.hpp>
#include <render/renderer.hpp>
#include <scene/scene.hpp>
#include <script/script.hpp>
#include <utility/singleton.hpp>
#include <window/window.hpp>

#include "get_system.hpp"

#ifdef EMSCRIPTEN
#include <emscripten.h>

std::function<void()> emscripten_loop;
void main_loop() { emscripten_loop(); }
#endif

namespace sinen {
int main::activate() {
  if (!singleton<main_system>::get().initialize()) {
    return -1;
  }
  singleton<main_system>::get().launch();
  return 0;
}
void main::change_scene(const std::string &scene_number) {
  singleton<main_system>::get().change_scene<scene>(scene_number);
}
std::string main::get_current_scene_number() {
  return singleton<main_system>::get().get_current_scene_number();
}
bool main_system::initialize() {
  logger::change_logger(
      std::move(logger::default_logger::CreateConsoleLogger()));
  logger::info("MAIN SYSTEM Activating");
  m_scene = std::make_unique<scene>();
  SDL_SetMainReady();
  SDL_Init(SDL_INIT_EVERYTHING);
  TTF_Init();
  IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
  SDLNet_Init();
  Mix_Init(MIX_INIT_OGG | MIX_INIT_MP3);
  Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
  std::ifstream ifs("./api");
  std::string str;
  if (ifs.fail())
    str = "Vulkan";
  std::getline(ifs, str);
  if (str.compare("Vulkan") == 0) {
    get_window().initialize(
        "sinen engine version:0.0.1, Graphics backends:Vulkan",
        graphics_api::Vulkan);
    get_renderer().initialize(graphics_api::Vulkan);
  } else if (str.compare("OpenGL") == 0) {
    get_window().initialize(
        "sinen engine version:0.0.1, Graphics backends:OpenGL",
        graphics_api::OpenGL);
    get_renderer().initialize(graphics_api::OpenGL);
  }

#else
  get_window().initialize("sinen engine version:0.0.1", graphics_api::ES);
  get_renderer().initialize(graphics_api::ES);
#endif
  singleton<camera>::get();
  if (!get_sound().initialize()) {
    logger::fatal("Failed to initialize audio system");
    get_sound().terminate();
    return false;
  }
  if (!get_input().initialize()) {
    logger::fatal("Failed to initialize input system");
    return false;
  }
  if (!get_script().initialize()) {
    logger::fatal("Failed to initialize script system");
    return false;
  }
  get_random().init();
  texture tex;
  tex.fill_color(palette::black());
  get_renderer().set_skybox_texture(tex);
  return true;
}
void main_system::launch() {
  m_scene->initialize();

#if !defined(EMSCRIPTEN)
  while (true)
    loop();
#else
  emscripten_loop = [&]() { loop(); };
  emscripten_set_main_loop(main_loop, 120, true);
#endif
}
void main_system::loop() {
  if (m_scene->is_running()) {
    m_scene->run_loop();
    return;
  }
  if (this->is_reset) {
    m_scene->initialize();
    this->is_reset = false;
    return;
  }
  logger::info("MAIN SYSTEM Inactiviating");
  get_script().shutdown();
  m_scene->shutdown();
  get_input().terminate();
  get_sound().terminate();
  get_renderer().shutdown();
  Mix_CloseAudio();
  singleton_finalizer::finalize();
#ifndef EMSCRIPTEN
  std::exit(0);
#else
  emscripten_force_exit(0);
#endif
}
void main_system::change_scene_impl(const std::string &scene_name) {
  m_scene->quit();
  get_script().shutdown();
  m_scene_name = scene_name;
  is_reset = true;
}
} // namespace sinen
