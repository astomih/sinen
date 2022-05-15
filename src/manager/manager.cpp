#ifndef MOBILE
#define SDL_MAIN_HANDLED
#else
#include <SDL_main.h>
#endif

#include "../texture/texture_system.hpp"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_net.h>
#include <SDL_ttf.h>
#include <fstream>
#include <memory>

#include <audio/sound_system.hpp>
#include <camera/camera.hpp>
#include <input/input_system.hpp>
#include <logger/logger.hpp>
#include <math/random.hpp>
#include <render/renderer.hpp>
#include <scene/scene.hpp>
#include <script/script.hpp>
#include <utility/singleton.hpp>
#include <window/window.hpp>

#ifdef EMSCRIPTEN
#include <emscripten.h>

std::function<void()> emscripten_loop;
void main_loop() { emscripten_loop(); }
#endif

namespace nen {
manager _manager;
bool initialize() { return _manager.initialize(); }
void launch() { _manager.launch(); }
window &get_window() { return _manager.get_window(); }
renderer &get_renderer() { return _manager.get_renderer(); }
input_system &get_input() { return _manager.get_input_system(); }
scene &get_current_scene() { return _manager.get_current_scene(); }
sound_system &get_sound() { return _manager.get_sound_system(); }
script_system &get_script() { return _manager.get_script_system(); }
texture_system &get_texture() { return _manager.get_texture_system(); }
camera &get_camera() { return _manager.get_camera(); }
random &get_random() { return _manager.get_random(); }
bool manager::initialize() {
  m_current_scene = std::make_unique<scene>(*this);
  SDL_SetMainReady();
  SDL_Init(SDL_INIT_EVERYTHING);
  TTF_Init();
  IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
  SDLNet_Init();
  Mix_Init(MIX_INIT_OGG | MIX_INIT_MP3);
  Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
  m_next_scene = nullptr;
  m_window = std::make_unique<nen::window>();
  m_texture_system = std::make_unique<nen::texture_system>(*this);
  m_renderer = std::make_unique<nen::renderer>(*this);
  nen::logger::change_logger(
      std::move(nen::logger::default_logger::CreateConsoleLogger()));
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
  std::ifstream ifs("./api");
  std::string str;
  if (ifs.fail())
    str = "Vulkan";
  std::getline(ifs, str);
  if (str.compare("Vulkan") == 0) {
    m_window->Initialize("Nen : Vulkan", nen::graphics_api::Vulkan);
    m_renderer->initialize(nen::graphics_api::Vulkan);
  } else if (str.compare("OpenGL") == 0) {
    m_window->Initialize("Nen : OpenGL", nen::graphics_api::OpenGL);
    m_renderer->initialize(nen::graphics_api::OpenGL);
  }

#else
  m_window->Initialize("Nen", nen::graphics_api::ES);
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
  m_input_system = std::make_unique<nen::input_system>(*this);
  if (!m_input_system->initialize()) {
    logger::fatal("Failed to initialize input system");
    return false;
  }
  m_script_system = std::make_unique<nen::script_system>(*this);
  if (!m_script_system->initialize()) {
    logger::fatal("Failed to initialize script system");
    return false;
  }
  m_random = std::make_unique<nen::random>();
  m_random->init();
  texture tex;
  tex.fill_color(palette::Black);
  m_renderer->set_skybox_texture(tex);
  return true;
}
void manager::launch() {
  m_current_scene->Initialize();

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
    m_current_scene->Initialize();
    m_next_scene = nullptr;
  } else {
    m_current_scene->Shutdown();
    m_input_system->terminate();
    m_sound_system->terminate();
    m_current_scene = nullptr;
    m_renderer->shutdown();
    m_renderer = nullptr;
    m_window = nullptr;
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
  m_next_scene = std::make_unique<nen::scene>(*this);
  m_scene_name = scene_name;
}

void change_scene(std::string scene_number) {
  _manager.change_scene(scene_number);
}

std::string get_current_scene_number() {
  return _manager.get_current_scene_number();
}

} // namespace nen
