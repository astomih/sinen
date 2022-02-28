#ifndef MOBILE
#define SDL_MAIN_HANDLED
#else
#include <SDL_main.h>
#endif

#include "../Texture/texture_system.hpp"
#include <Nen.hpp>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_net.h>
#include <SDL_ttf.h>
#include <fstream>
#include <memory>

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
input_system &get_input_system() { return _manager.get_input_system(); }
scene &get_current_scene() { return _manager.get_current_scene(); }
sound_system &get_sound_system() { return _manager.get_sound_system(); }
script_system &get_script_system() { return _manager.get_script_system(); }
texture_system &get_texture_system() { return _manager.get_texture_system(); }
font_system &get_font_system() { return _manager.get_font_system(); }
bool manager::initialize() {
  m_current_scene = std::make_unique<scene>(*this);
  SDL_SetMainReady();
  SDL_Init(SDL_INIT_EVERYTHING);
  TTF_Init();
  IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
  SDLNet_Init();
  m_next_scene = nullptr;
  m_window = std::make_unique<nen::window>();
  m_renderer = std::make_unique<nen::renderer>(*this);
  nen::logger::MakeLogger(
      std::move(nen::logger::default_logger::CreateConsoleLogger()));
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
  std::ifstream ifs("./api");
  std::string str;
  if (ifs.fail())
    str = "Vulkan";
  std::getline(ifs, str);
  if (str.compare("Vulkan") == 0) {
    m_window->Initialize(nen::vector2(1280, 720), "Nen : Vulkan",
                         nen::graphics_api::Vulkan);
    m_renderer->initialize(nen::graphics_api::Vulkan);
  } else if (str.compare("OpenGL") == 0) {
    m_window->Initialize(nen::vector2(1280, 720), "Nen : OpenGL",
                         nen::graphics_api::OpenGL);
    m_renderer->initialize(nen::graphics_api::OpenGL);
  }

#else
  m_window->Initialize(nen::vector2(1280, 720), "Nen", nen::graphics_api::ES);
  m_renderer->initialize(nen::graphics_api::ES);
#endif
  m_renderer->SetProjectionMatrix(nen::matrix4::Perspective(
      nen::Math::ToRadians(70.f), m_window->Size().x / m_window->Size().y, 0.1f,
      1000.f));
  m_renderer->SetViewMatrix(
      nen::matrix4::LookAt(vector3(0, 0, 10), vector3(0), vector3(0, 1, 0)));

  m_sound_system = std::make_unique<nen::sound_system>();
  if (!m_sound_system->Initialize()) {
    nen::logger::Info("Failed to initialize audio system");
    m_sound_system->Shutdown();
    m_sound_system = nullptr;
    return false;
  }
  m_input_system = std::make_unique<nen::input_system>(*this);
  if (!m_input_system->Initialize()) {
    nen::logger::Info("Failed to initialize input system");
    return false;
  }
  m_script_system = std::make_unique<nen::script_system>(*this);
  if (!m_script_system->initialize()) {
    nen::logger::Info("Failed to initialize script system");
    return false;
  }
  m_texture_system = std::make_unique<nen::texture_system>(*this);

  m_font_system = std::make_unique<nen::font_system>(*this);
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
    m_input_system->Shutdown();
    m_sound_system->Shutdown();
    m_current_scene = nullptr;
    m_renderer->Shutdown();
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
void manager::change_scene(std::uint32_t scene_number) {
  m_current_scene->Quit();
  m_next_scene = std::make_unique<nen::scene>(*this);
  m_scene_number = scene_number;
}

} // namespace nen