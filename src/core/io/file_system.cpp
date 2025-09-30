#include "core/logger/logger.hpp"

#include <core/io/file_system.hpp>

#include <SDL3/SDL.h>

namespace sinen {
std::vector<std::string>
FileSystem::enumerateDirectory(const std::string &path) {
  auto p = getAppBaseDirectory() + "/" + path;
  std::vector<std::string> result;
  SDL_EnumerateDirectory(
      p.c_str(),
      [](void *userdata, const char *dirname, const char *fname) {
        auto *vec = reinterpret_cast<std::vector<std::string> *>(userdata);
        vec->emplace_back(fname);
        if (fname == nullptr)
          return SDL_ENUM_SUCCESS;
        return SDL_ENUM_CONTINUE;
      },
      &result);
  return result;
}
std::string FileSystem::getAppBaseDirectory() {

#ifdef __ANDROID__
  std::string base = "/sdcard/Android/media/org.libsdl.app";
#else
  std::string base = ".";
#endif
  return base;
}
} // namespace sinen