#ifndef MOBILE
#define SDL_MAIN_HANDLED
#else
#include <SDL_main.h>
#endif

#include "manager.hpp"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_net.h>
#include <SDL_ttf.h>
#include <fstream>
#include <memory>
#include <optional>
#include <utility/launcher.hpp>

#include <camera/camera.hpp>
#include <input/input.hpp>
#include <logger/logger.hpp>
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
bool initialize() { return singleton<manager>::get().initialize(); }
void launch() { singleton<manager>::get().launch(); }
window_system &get_window() { return singleton<window_system>::get(); }
render_system &get_renderer() { return singleton<render_system>::get(); }
input_system &get_input() { return singleton<input_system>::get(); }
scene &get_current_scene() {
  return singleton<manager>::get().get_current_scene();
}
sound_system &get_sound() { return singleton<sound_system>::get(); }
texture_system &get_texture() { return singleton<texture_system>::get(); }
camera &get_camera() { return singleton<camera>::get(); }
random_system &get_random() { return singleton<random_system>::get(); }
script_system &get_script() { return singleton<script_system>::get(); }
event_system &get_event() { return singleton<event_system>::get(); }
bool manager::initialize() {
  m_current_scene = std::make_unique<scene>();
  m_next_scene = nullptr;
  SDL_SetMainReady();
  SDL_Init(SDL_INIT_EVERYTHING);
  TTF_Init();
  IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
  SDLNet_Init();
  Mix_Init(MIX_INIT_OGG | MIX_INIT_MP3);
  Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
  logger::change_logger(
      std::move(logger::default_logger::CreateConsoleLogger()));
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
void manager::launch() {
  m_current_scene->initialize();

#if !defined(EMSCRIPTEN)
  while (true)
    loop();
#else
  emscripten_loop = [&]() { loop(); };
  emscripten_set_main_loop(main_loop, 120, true);
#endif
}
void manager::loop() {
  if (m_current_scene->is_running())
    m_current_scene->run_loop();
  else if (m_next_scene) {
    m_current_scene->shutdown();
    m_current_scene = std::move(m_next_scene);
    m_current_scene->initialize();
    m_next_scene = nullptr;
  } else {
    get_script().shutdown();
    m_current_scene->shutdown();
    get_input().terminate();
    get_sound().terminate();
    m_current_scene = nullptr;
    get_renderer().shutdown();
    Mix_CloseAudio();
    singleton_finalizer::finalize();
#ifndef EMSCRIPTEN
    std::exit(0);
#else
    emscripten_force_exit(0);
#endif
  }
}
void manager::change_scene(std::string scene_name) {
  m_current_scene->quit();
  get_script().shutdown();
  m_next_scene = std::make_unique<scene>();
  m_scene_name = scene_name;
}

void change_scene(std::string scene_number) {
  singleton<manager>::get().change_scene(scene_number);
}

std::string get_current_scene_number() {
  return singleton<manager>::get().get_current_scene_number();
}

} // namespace sinen
