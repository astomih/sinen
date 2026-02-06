#include <audio/audio.hpp>
#include <core/event/event.hpp>
#include <core/logger/log.hpp>
#include <core/time/time.hpp>
#include <graphics/graphics.hpp>
#include <math/random.hpp>
#include <physics/physics.hpp>
#include <platform/input/input.hpp>
#include <platform/io/arguments.hpp>
#include <platform/window/window.hpp>
#include <script/script.hpp>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#include <SDL3/SDL.h>

using namespace sinen;

#include <imgui.h>
#include <imgui_impl_sdl3.h>

static inline bool addOverflowSize(size_t a, size_t b, size_t *out) {
  if (a > SIZE_MAX - b)
    return true;
  *out = a + b;
  return false;
}

static inline bool mulOverflowSize(size_t a, size_t b, size_t *out) {
  if (a != 0 && b > SIZE_MAX / a)
    return true;
  *out = a * b;
  return false;
}

static void *mallocCustom(size_t size) {
  size_t totalSize;
  if (addOverflowSize(size, sizeof(Size), &totalSize))
    return nullptr;

  auto *m = static_cast<Size *>(GlobalAllocator::get()->allocate(totalSize));
  if (!m)
    return nullptr;

  *m = totalSize;
  return static_cast<void *>(m + 1);
}

static void *callocCustom(size_t nmemb, size_t size) {
  size_t payload;
  if (mulOverflowSize(nmemb, size, &payload))
    return nullptr;

  size_t totalSize;
  if (addOverflowSize(payload, sizeof(Size), &totalSize))
    return nullptr;

  auto *m = static_cast<Size *>(GlobalAllocator::get()->allocate(totalSize));
  if (!m)
    return nullptr;

  SDL_memset(m, 0, totalSize);
  *m = totalSize;
  return static_cast<void *>(m + 1);
}

static void freeCustom(void *mem);
static void *reallocCustom(void *src, size_t size) {
  if (!src)
    return mallocCustom(size);

  if (size == 0) {
    freeCustom(src);
    return nullptr;
  }

  // Get old header
  auto *oldHeader = static_cast<Size *>(src) - 1;
  size_t oldTotal = *oldHeader;
  size_t oldPayload =
      (oldTotal >= sizeof(Size)) ? (oldTotal - sizeof(Size)) : 0;

  size_t newTotal;
  if (addOverflowSize(size, sizeof(Size), &newTotal))
    return nullptr;

  auto *newHeader =
      static_cast<Size *>(GlobalAllocator::get()->allocate(newTotal));
  if (!newHeader) {
    return nullptr;
  }

  *newHeader = newTotal;
  void *dst = static_cast<void *>(newHeader + 1);

  size_t copyBytes = std::min(oldPayload, size);
  if (copyBytes > 0)
    SDL_memcpy(dst, src, copyBytes);

  GlobalAllocator::get()->deallocate(static_cast<void *>(oldHeader), oldTotal);

  return dst;
}

static void freeCustom(void *mem) {
  if (!mem)
    return;

  auto *m = static_cast<Size *>(mem) - 1;
  GlobalAllocator::get()->deallocate(static_cast<void *>(m), *m);
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
  SDL_SetMemoryFunctions(mallocCustom, callocCustom, reallocCustom, freeCustom);
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
