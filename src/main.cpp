#include <audio/audio.hpp>
#include <core/allocator/engine_memory.hpp>
#include <core/event/event.hpp>
#include <core/logger/log.hpp>
#include <core/profiler.hpp>
#include <core/time/time.hpp>
#include <graphics/graphics.hpp>
#include <math/random.hpp>
#include <physics/physics.hpp>
#include <platform/input/input.hpp>
#include <platform/io/asset_reader.hpp>
#include <platform/window/window.hpp>
#include <script/script.hpp>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#include <SDL3/SDL.h>

#include <algorithm>
#include <cstddef>
#include <cstring>

using namespace sinen;

namespace {
struct alignas(std::max_align_t) AllocationHeader {
  Size totalSize;
};

inline bool addOverflowSize(size_t a, size_t b, size_t *out) {
  if (a > SIZE_MAX - b)
    return true;
  *out = a + b;
  return false;
}

inline bool mulOverflowSize(size_t a, size_t b, size_t *out) {
  if (a != 0 && b > SIZE_MAX / a)
    return true;
  *out = a * b;
  return false;
}

void *mallocCustom(size_t size) {
  size_t totalSize;
  if (addOverflowSize(size, sizeof(AllocationHeader), &totalSize))
    return nullptr;

  auto *m = static_cast<AllocationHeader *>(
      EngineMemory::global()->allocate(totalSize));
  if (!m)
    return nullptr;

  m->totalSize = totalSize;
  return static_cast<void *>(m + 1);
}

void *callocCustom(size_t nmemb, size_t size) {
  size_t payload;
  if (mulOverflowSize(nmemb, size, &payload))
    return nullptr;

  size_t totalSize;
  if (addOverflowSize(payload, sizeof(AllocationHeader), &totalSize))
    return nullptr;

  auto *m = static_cast<AllocationHeader *>(
      EngineMemory::global()->allocate(totalSize));
  if (!m)
    return nullptr;

  SDL_memset(m, 0, totalSize);
  m->totalSize = totalSize;
  return static_cast<void *>(m + 1);
}

void freeCustom(void *mem);
void *reallocCustom(void *src, size_t size) {
  if (!src)
    return mallocCustom(size);

  if (size == 0) {
    freeCustom(src);
    return nullptr;
  }

  // Get old header
  auto *oldHeader = static_cast<AllocationHeader *>(src) - 1;
  size_t oldTotal = oldHeader->totalSize;
  size_t oldPayload = (oldTotal >= sizeof(AllocationHeader))
                          ? (oldTotal - sizeof(AllocationHeader))
                          : 0;

  size_t newTotal;
  if (addOverflowSize(size, sizeof(AllocationHeader), &newTotal))
    return nullptr;

  auto *newHeader = static_cast<AllocationHeader *>(
      EngineMemory::global()->allocate(newTotal));
  if (!newHeader) {
    return nullptr;
  }

  newHeader->totalSize = newTotal;
  void *dst = static_cast<void *>(newHeader + 1);

  size_t copyBytes = std::min(oldPayload, size);
  if (copyBytes > 0)
    SDL_memcpy(dst, src, copyBytes);

  EngineMemory::global()->deallocate(static_cast<void *>(oldHeader), oldTotal);

  return dst;
}

void freeCustom(void *mem) {
  if (!mem)
    return;

  auto *m = static_cast<AllocationHeader *>(mem) - 1;
  EngineMemory::global()->deallocate(static_cast<void *>(m), m->totalSize);
}

String getWindowTitleWithBackend(GPUBackendAPI api) {
  return "Sinen | " + Graphics::getBackendName(api) +
         " | F8: Switch API | F11: Fullscreen";
}

} // namespace

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
  ZoneScopedN("SDL_AppInit");
  if (!EngineMemory::initialize()) {
    Log::critical("Failed to initialize engine memory");
    return SDL_APP_FAILURE;
  }
  std::pmr::set_default_resource(EngineMemory::global());
  SDL_SetMemoryFunctions(mallocCustom, callocCustom, reallocCustom, freeCustom);
  for (int i = 1; i < argc; i++) {
    StringView arg(argv[i]);
    if (arg.size() >= std::strlen(AssetReader::archiveExtension()) &&
        arg.substr(arg.size() - std::strlen(AssetReader::archiveExtension())) ==
            AssetReader::archiveExtension()) {
      AssetReader::mountArchive(arg);
      break;
    }
  }

  SDL_SetHint(SDL_HINT_APP_NAME, "Sinen");
  SDL_SetHint(SDL_HINT_ANDROID_TRAP_BACK_BUTTON, "1");
  SDL_InitFlags initFlags = SDL_INIT_AUDIO | SDL_INIT_EVENTS;
  if (!SDL_Init(initFlags)) {
    Log::critical("Failed to initialize SDL");
    return SDL_APP_FAILURE;
  }
  GPUBackendAPI api = Graphics::chooseBackendApiByPlatformFeatures();
  if (!Window::initialize(getWindowTitleWithBackend(api))) {
    Log::critical("Failed to initialize window");
    return SDL_APP_FAILURE;
  }
  if (!Graphics::initialize(api, EngineMemory::graphics())) {
    Log::critical("Failed to initialize graphics");
    return SDL_APP_FAILURE;
  }
  if (!Audio::initialize()) {
    Log::critical("Failed to initialize audio");
    Audio::shutdown();
#ifndef SINEN_PLATFORM_EMSCRIPTEN
    return SDL_APP_FAILURE;
#endif
  }
  if (!Input::initialize()) {
    Log::critical("Failed to initialize input");
    return SDL_APP_FAILURE;
  }
  if (!Physics::initialize()) {
    Log::critical("Failed to initialize physics");
    return SDL_APP_FAILURE;
  }
  if (!Random::initialize()) {
    Log::critical("Failed to initialize random");
    return SDL_APP_FAILURE;
  }
  if (!Script::initialize()) {
    Log::critical("Failed to initialize script");
    return SDL_APP_FAILURE;
  }
  EngineMemory::resetScene();
  Script::executeScene();
  return SDL_APP_CONTINUE;
}
SDL_AppResult SDL_AppIterate(void *appstate) {
  ZoneScopedN("Frame");
  EngineMemory::beginFrame();
  if (Event::isQuit()) {
    EngineMemory::endFrame();
    return SDL_APP_SUCCESS;
  }
  if (Event::isPaused()) {
    EngineMemory::endFrame();
    return SDL_APP_CONTINUE;
  }
  {
    ZoneScopedN("Input::update");
    Input::update();
  }
  {
    ZoneScopedN("Time::update");
    Time::update();
  }
  {
    ZoneScopedN("Script::updateScene");
    Script::callUpdate();
  }
  Graphics::render();
  if (Script::hasToReloadScene()) {
    ZoneScopedN("Script::runScene");
    EngineMemory::resetScene();
    Script::executeScene();
  }
  if (Keyboard::isPressed(Scancode::F8)) {
    auto currentBackendAPI = Graphics::getBackendAPI();
    Script::shutdown();
    Graphics::shutdown();

    EngineMemory::resetScene();
    Script::initialize();

    auto api = currentBackendAPI;
    auto getNextAPI = [](GPUBackendAPI api) {
      return static_cast<GPUBackendAPI>(
          (static_cast<Int32>(api) + 1) %
          static_cast<Int32>(GPUBackendAPI::COUNT));
    };
    api = getNextAPI(api);
    while (!Graphics::initialize(api, EngineMemory::graphics())) {
      api = getNextAPI(api);
      if (api == currentBackendAPI) {
        Log::critical("Failed to initialize any graphics backend.");
        return SDL_APP_FAILURE;
      }
    }
    Window::rename(getWindowTitleWithBackend(api));
    Script::executeScene();
  }
  if (Keyboard::isPressed(Scancode::F11)) {
    static bool fullscreen = false;
    fullscreen = !fullscreen;
    Window::setFullscreen(fullscreen);
  }
  Window::prepareFrame();
  Input::prepareForUpdate();
  EngineMemory::endFrame();
  FrameMark;
  return SDL_APP_CONTINUE;
}
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  ZoneScopedN("SDL_AppEvent");
  Event::processEvent(*event);
  Window::processEvent(*event);
  Input::processEvent(*event);
  return SDL_APP_CONTINUE;
}
void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  ZoneScopedN("SDL_AppQuit");
  Script::shutdown();
  AssetReader::unmountArchive();
  Physics::shutdown();
  Input::shutdown();
  Audio::shutdown();
  Random::shutdown();
  Graphics::shutdown();
  Window::shutdown();
  EngineMemory::shutdown();
}
