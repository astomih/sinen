#include "core/logger/log.hpp"

#include <platform/io/filesystem.hpp>

#include <SDL3/SDL.h>

namespace sinen {
Array<String> Filesystem::enumerateDirectory(StringView path) {
  auto p = getAppBaseDirectory() + "/" + String(path.data());
  Array<String> result;
  SDL_EnumerateDirectory(
      p.c_str(),
      [](void *userdata, const char *dirname, const char *fname) {
        auto *vec = reinterpret_cast<Array<String> *>(userdata);
        vec->emplace_back(fname);
        if (fname == nullptr)
          return SDL_ENUM_SUCCESS;
        return SDL_ENUM_CONTINUE;
      },
      &result);
  return result;
}
String Filesystem::getAppBaseDirectory() {

#ifdef __ANDROID__
  String base = "/sdcard/Android/media/astomih.sinen.app";
#else
  String base = ".";
#endif
  return base;
}
} // namespace sinen