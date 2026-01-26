#include <asset/audio/audio.hpp>
#include <asset/script/script.hpp>
#include <core/event/event.hpp>
#include <graphics/graphics.hpp>
#include <math/random.hpp>
#include <physics/physics.hpp>
#include <platform/input/input.hpp>
#include <platform/io/arguments.hpp>
#include <platform/window/window.hpp>

#include <sinen.hpp>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#include <SDL3/SDL.h>

using namespace sinen;

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <iostream>

void *mallocCustom(size_t size) {
  Size *m = (Size *)GlobalAllocator::get()->allocate(size + sizeof(Size));
  std::cout << "malloc: " << m << std::endl;
  *m = size + sizeof(Size);
  void *mem = m + 1;
  return mem;
}
void *callocCustom(size_t nmemb, size_t size) {
  Size *m =
      (Size *)GlobalAllocator::get()->allocate(size * nmemb + sizeof(Size));
  std::cout << "calloc: " << m << std::endl;
  SDL_memset(m, 0, size * nmemb);
  *m = size * nmemb + sizeof(Size);
  void *mem = m + 1;
  return mem;
}

void *reallocCustom(void *src, size_t size) {
  Size *m = (Size *)GlobalAllocator::get()->allocate(size + sizeof(Size));
  *m = size + sizeof(Size);
  void *mem = m + 1;
  SDL_memcpy(mem, src, *((Size *)(src)-1));
  return mem;
}

void freeCustom(void *mem) {
  Size *m = (Size *)mem;
  m = m - 1;
  std::cout << "free: " << m << std::endl;
  GlobalAllocator::get()->deallocate((void *)m, *m);
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
  Arguments::argc = argc;
  Arguments ::argv.resize(argc);
  for (int i = 0; i < argc; i++) {
    Arguments::argv[i] = argv[i];
  }

  SDL_SetHint(SDL_HINT_APP_NAME, "Sinen");
  SDL_SetHint(SDL_HINT_ANDROID_TRAP_BACK_BUTTON, "1");
  SDL_InitFlags initFlags = SDL_INIT_AUDIO | SDL_INIT_EVENTS;
  if (!SDL_Init(initFlags)) {
    Log::critical("Failed to initialize SDL");
    return SDL_APP_FAILURE;
  }

  if (!Window::initialize("Sinen")) {
    Log::critical("Failed to initialize window");
    return SDL_APP_FAILURE;
  }
  if (!Graphics::initialize()) {
    Log::critical("Failed to initialize graphics");
    return SDL_APP_FAILURE;
  }
  if (!Audio::initialize()) {
    Log::critical("Failed to initialize audio");
    Audio::shutdown();
    return SDL_APP_FAILURE;
  }
  if (!Input::initialize()) {
    Log::critical("Failed to initialize input");
    return SDL_APP_FAILURE;
  }
  if (!Physics::initialize()) {
    Log::critical("Failed to initialize physics");
    return SDL_APP_FAILURE;
  }
  if (!Script::initialize()) {
    Log::critical("Failed to initialize script");
    return SDL_APP_FAILURE;
  }
  if (!Random::initialize()) {
    Log::critical("Failed to initialize random");
    return SDL_APP_FAILURE;
  }
  return SDL_APP_CONTINUE;
}
SDL_AppResult SDL_AppIterate(void *appstate) {
  if (Event::isQuit())
    return SDL_APP_SUCCESS;
  Input::update();
  Time::update();
  Script::updateScene();
  Physics::update();
  Graphics::render();
  if (Script::hasToReload()) {
    Script::runScene();
    Physics::postSetup();
    Script::doneReload();
  }
  if (Keyboard::isPressed(Keyboard::Code::F11)) {
    static bool fullscreen = false;
    fullscreen = !fullscreen;
    Window::setFullscreen(fullscreen);
  }
  Window::prepareFrame();
  Input::prepareForUpdate();
  return SDL_APP_CONTINUE;
}
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  ImGui_ImplSDL3_ProcessEvent(event);
  Event::processEvent(*event);
  Window::processEvent(*event);
  Input::processEvent(*event);
  return SDL_APP_CONTINUE;
}
void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  Physics::shutdown();
  Input::shutdown();
  Audio::shutdown();
  Random::shutdown();
  Graphics::shutdown();
  Window::shutdown();
  Script::shutdown();
  SDL_Quit();
}
