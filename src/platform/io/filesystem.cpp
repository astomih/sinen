#include "core/logger/log.hpp"

#include <platform/io/filesystem.hpp>
#include <script/luaapi.hpp>

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

static int lFilesystemEnumerateDirectory(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  auto list = Filesystem::enumerateDirectory(StringView(path));
  lua_newtable(L);
  for (size_t i = 0; i < list.size(); ++i) {
    lua_pushlstring(L, list[i].data(), list[i].size());
    lua_rawseti(L, -2, static_cast<lua_Integer>(i + 1));
  }
  return 1;
}
void registerFilesystem(lua_State *L) {
  pushSnNamed(L, "Filesystem");
  luaPushcfunction2(L, lFilesystemEnumerateDirectory);
  lua_setfield(L, -2, "enumerateDirectory");
  lua_pop(L, 1);
}
} // namespace sinen
