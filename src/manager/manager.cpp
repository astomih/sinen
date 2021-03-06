#ifndef MOBILE
#define SDL_MAIN_HANDLED
#else
#include <SDL_main.h>
#endif

#include "../input/input_system.hpp"
#include "../math/random_system.hpp"
#include "../render/render_system.hpp"
#include "../script/script_system.hpp"
#include "../texture/texture_system.hpp"
#include "../window/window_system.hpp"
#include "manager.hpp"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_net.h>
#include <SDL_ttf.h>
#include <fstream>
#include <memory>
#include <utility/launcher.hpp>

#include "../audio/sound_system.hpp"
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

namespace nen {
manager _manager;
std::unique_ptr<class window_system> m_window;
std::unique_ptr<class render_system> m_renderer;
std::unique_ptr<class scene> m_next_scene;
std::unique_ptr<class input_system> m_input_system;
std::unique_ptr<class sound_system> m_sound_system;
std::unique_ptr<class script_system> m_script_system;
std::unique_ptr<class texture_system> m_texture_system;
std::unique_ptr<class camera> m_camera;
std::unique_ptr<class random_system> m_random;
bool initialize() { return _manager.initialize(); }
void launch() { _manager.launch(); }
window_system &get_window() { return *m_window; }
render_system &get_renderer() { return *m_renderer; }
input_system &get_input() { return *m_input_system; }
scene &get_current_scene() { return _manager.get_current_scene(); }
sound_system &get_sound() { return *m_sound_system; }
texture_system &get_texture() { return *m_texture_system; }
camera &get_camera() { return *m_camera; }
random_system &get_random() { return *m_random; }
script_system &get_script() { return *m_script_system; }
bool manager::initialize() {
  m_current_scene = std::make_unique<scene>();
  SDL_SetMainReady();
  SDL_Init(SDL_INIT_EVERYTHING);
  TTF_Init();
  IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
  SDLNet_Init();
  Mix_Init(MIX_INIT_OGG | MIX_INIT_MP3);
  Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
  m_next_scene = nullptr;
  m_window = std::make_unique<window_system>();
  m_texture_system = std::make_unique<texture_system>();
  m_renderer = std::make_unique<nen::render_system>();
  nen::logger::change_logger(
      std::move(nen::logger::default_logger::CreateConsoleLogger()));
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
  std::ifstream ifs("./api");
  std::string str;
  if (ifs.fail())
    str = "Vulkan";
  std::getline(ifs, str);
  if (str.compare("Vulkan") == 0) {
    m_window->initialize("Nen : Vulkan", nen::graphics_api::Vulkan);
    m_renderer->initialize(nen::graphics_api::Vulkan);
  } else if (str.compare("OpenGL") == 0) {
    m_window->initialize("Nen : OpenGL", nen::graphics_api::OpenGL);
    m_renderer->initialize(nen::graphics_api::OpenGL);
  }

#else
  m_window->initialize("Nen", nen::graphics_api::ES);
  m_renderer->initialize(nen::graphics_api::ES);
#endif
  m_camera = std::make_unique<camera>();

  m_sound_system = std::make_unique<nen::sound_system>();
  if (!m_sound_system->initialize()) {
    logger::fatal("Failed to initialize audio system");
    m_sound_system->terminate();
    m_sound_system = nullptr;
    return false;
  }
  m_input_system = std::make_unique<nen::input_system>();
  if (!m_input_system->initialize()) {
    logger::fatal("Failed to initialize input system");
    return false;
  }
  m_script_system = std::make_unique<nen::script_system>();
  if (!m_script_system->initialize()) {
    logger::fatal("Failed to initialize script system");
    return false;
  }
  m_random = std::make_unique<nen::random_system>();
  m_random->init();
  texture tex;
  tex.fill_color(palette::Black);
  m_renderer->set_skybox_texture(tex);
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
  if (m_current_scene->isRunning())
    m_current_scene->RunLoop();
  else if (m_next_scene) {
    m_current_scene->Shutdown();
    m_current_scene = std::move(m_next_scene);
    m_current_scene->initialize();
    m_next_scene = nullptr;
  } else {
    m_script_system->shutdown();
    m_current_scene->Shutdown();
    m_input_system->terminate();
    m_sound_system->terminate();
    m_current_scene = nullptr;
    m_renderer->shutdown();
    m_renderer = nullptr;
    m_window = nullptr;
    Mix_CloseAudio();
    singleton_finalizer::Finalize();
#ifndef EMSCRIPTEN
    std::exit(0);
#else
    emscripten_force_exit(0);
#endif
  }
}
void manager::change_scene(std::string scene_name) {
  m_current_scene->Quit();
  m_script_system->shutdown();
  m_next_scene = std::make_unique<nen::scene>();
  m_scene_name = scene_name;
}

void change_scene(std::string scene_number) {
  _manager.change_scene(scene_number);
}

std::string get_current_scene_number() {
  return _manager.get_current_scene_number();
}

} // namespace nen
