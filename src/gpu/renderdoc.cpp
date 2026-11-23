#include <gpu/renderdoc.hpp>

#include <SDL3/SDL.h>
#include <core/logger/log.hpp>

#if defined(_WIN32) || (defined(__linux__) && !defined(__EMSCRIPTEN__))
#include <renderdoc_app.h>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace {
RENDERDOC_API_1_1_2 *api = nullptr;
// Never unload RenderDoc: its hooks can outlive the graphics device.
SDL_SharedObject *loadedLibrary = nullptr;

pRENDERDOC_GetAPI findGetAPI() {
#if defined(_WIN32)
  if (auto module = GetModuleHandleW(L"renderdoc.dll")) {
    return reinterpret_cast<pRENDERDOC_GetAPI>(
        GetProcAddress(module, "RENDERDOC_GetAPI"));
  }
#else
#if defined(__ANDROID__)
  constexpr auto libraryName = "libVkLayer_GLES_RenderDoc.so";
#else
  constexpr auto libraryName = "librenderdoc.so";
#endif
  // RTLD_NOLOAD only finds injected libraries; it does not load RenderDoc.
  static void *module = nullptr;
  if (!module) {
    module = dlopen(libraryName, RTLD_NOW | RTLD_NOLOAD);
  }
  if (module) {
    return reinterpret_cast<pRENDERDOC_GetAPI>(
        dlsym(module, "RENDERDOC_GetAPI"));
  }
#endif
  const char *path = SDL_getenv("SINEN_RENDERDOC_LIBRARY");
  if (!loadedLibrary && path && *path) {
    loadedLibrary = SDL_LoadObject(path);
    if (!loadedLibrary) {
      sinen::Log::warn("Failed to load RenderDoc '{}': {}", path,
                       SDL_GetError());
    }
  }
  return loadedLibrary ? reinterpret_cast<pRENDERDOC_GetAPI>(SDL_LoadFunction(
                             loadedLibrary, "RENDERDOC_GetAPI"))
                       : nullptr;
}
} // namespace
#endif

namespace sinen {
bool RenderDoc::initialize() {
#if defined(_WIN32) || (defined(__linux__) && !defined(__EMSCRIPTEN__))
  if (api) {
    return true;
  }
  const auto getAPI = findGetAPI();
  if (!getAPI) {
    return false;
  }
  void *result = nullptr;
  if (getAPI(eRENDERDOC_API_Version_1_1_2, &result) != 1 || !result) {
    Log::warn("RenderDoc API 1.1.2 is unavailable");
    return false;
  }
  api = static_cast<RENDERDOC_API_1_1_2 *>(result);
  Log::info("RenderDoc API connected");
  return true;
#else
  return false;
#endif
}

bool RenderDoc::isAvailable() {
#if defined(_WIN32) || (defined(__linux__) && !defined(__EMSCRIPTEN__))
  return api != nullptr;
#else
  return false;
#endif
}

bool RenderDoc::triggerCapture(UInt32 numFrames) {
#if defined(_WIN32) || (defined(__linux__) && !defined(__EMSCRIPTEN__))
  if (api && numFrames > 0) {
    if (numFrames == 1) {
      api->TriggerCapture();
    } else {
      api->TriggerMultiFrameCapture(numFrames);
    }
    return true;
  }
#endif
  return false;
}

bool RenderDoc::isFrameCapturing() {
#if defined(_WIN32) || (defined(__linux__) && !defined(__EMSCRIPTEN__))
  return api && api->IsFrameCapturing() != 0;
#else
  return false;
#endif
}

bool RenderDoc::setCaptureFilePathTemplate(const char *path) {
#if defined(_WIN32) || (defined(__linux__) && !defined(__EMSCRIPTEN__))
  if (api && path && *path) {
    api->SetCaptureFilePathTemplate(path);
    return true;
  }
#endif
  return false;
}

UInt32 RenderDoc::getNumCaptures() {
#if defined(_WIN32) || (defined(__linux__) && !defined(__EMSCRIPTEN__))
  return api ? api->GetNumCaptures() : 0;
#else
  return 0;
#endif
}

String RenderDoc::getCaptureFilePath(UInt32 index) {
#if defined(_WIN32) || (defined(__linux__) && !defined(__EMSCRIPTEN__))
  UInt32 length = 0;
  if (api && api->GetCapture(index, nullptr, &length, nullptr) && length > 0) {
    String path(length, '\0');
    if (api->GetCapture(index, path.data(), &length, nullptr)) {
      path.resize(SDL_strlen(path.c_str()));
      return path;
    }
  }
#endif
  return {};
}
} // namespace sinen
