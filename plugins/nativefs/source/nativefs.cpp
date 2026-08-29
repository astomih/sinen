#include <nativefs/nativefs.hpp>

#include <lua.h>
#include <lualib.h>
#include <script/sinen_lua_module.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace sinen::plugins::nativefs {
namespace {

std::filesystem::path checkPath(lua_State *state, int index) {
  size_t size = 0;
  const char *value = luaL_checklstring(state, index, &size);
  const std::string_view path(value, size);
  if (path.find('\0') != std::string_view::npos) {
    luaL_error(state, "native filesystem path contains a null byte");
  }
  return std::filesystem::u8path(path.begin(), path.end());
}

std::string pathToUtf8(const std::filesystem::path &path) {
  const auto encoded = path.u8string();
  return std::string(reinterpret_cast<const char *>(encoded.data()),
                     encoded.size());
}

int pushError(lua_State *state, std::string message) {
  lua_pushnil(state);
  lua_pushlstring(state, message.data(), message.size());
  return 2;
}

int read(lua_State *state) {
  const std::filesystem::path path = checkPath(state, 1);
  std::ifstream stream(path, std::ios::binary | std::ios::ate);
  if (!stream) {
    return pushError(state, "cannot open file: " + pathToUtf8(path));
  }

  const std::streamoff length = stream.tellg();
  if (length < 0 ||
      static_cast<uintmax_t>(length) >
          static_cast<uintmax_t>(std::numeric_limits<size_t>::max()) ||
      static_cast<uintmax_t>(length) >
          static_cast<uintmax_t>(std::numeric_limits<std::streamsize>::max())) {
    return pushError(state, "cannot determine file size: " + pathToUtf8(path));
  }

  const auto size = static_cast<size_t>(length);
  std::string contents(size, '\0');
  stream.seekg(0, std::ios::beg);
  if (size > 0 &&
      !stream.read(contents.data(), static_cast<std::streamsize>(size))) {
    return pushError(state, "cannot read file: " + pathToUtf8(path));
  }

  lua_pushlstring(state, contents.data(), contents.size());
  lua_pushnil(state);
  return 2;
}

int exists(lua_State *state) {
  std::error_code error;
  const bool result = std::filesystem::exists(checkPath(state, 1), error);
  lua_pushboolean(state, result && !error);
  return 1;
}

int enumerateDirectory(lua_State *state) {
  const std::filesystem::path path = checkPath(state, 1);
  std::error_code error;
  std::filesystem::directory_iterator iterator(path, error);
  if (error) {
    return pushError(state, "cannot enumerate directory: " + error.message());
  }

  std::vector<std::string> entries;
  const std::filesystem::directory_iterator end;
  while (iterator != end) {
    entries.push_back(pathToUtf8(iterator->path().filename()));
    iterator.increment(error);
    if (error) {
      return pushError(state, "cannot enumerate directory: " + error.message());
    }
  }
  std::sort(entries.begin(), entries.end());

  lua_createtable(state, static_cast<int>(entries.size()), 0);
  for (size_t i = 0; i < entries.size(); ++i) {
    lua_pushlstring(state, entries[i].data(), entries[i].size());
    lua_rawseti(state, -2, static_cast<int>(i + 1));
  }
  lua_pushnil(state);
  return 2;
}

int getCurrentDirectory(lua_State *state) {
  std::error_code error;
  const std::filesystem::path path = std::filesystem::current_path(error);
  if (error) {
    return pushError(state, "cannot get current directory: " + error.message());
  }
  const std::string result = pathToUtf8(path);
  lua_pushlstring(state, result.data(), result.size());
  lua_pushnil(state);
  return 2;
}

int getAbsolutePath(lua_State *state) {
  std::error_code error;
  std::filesystem::path path =
      std::filesystem::absolute(checkPath(state, 1), error);
  if (error) {
    return pushError(state, "cannot resolve absolute path: " + error.message());
  }
  path = path.lexically_normal();
  const std::string result = pathToUtf8(path);
  lua_pushlstring(state, result.data(), result.size());
  lua_pushnil(state);
  return 2;
}

void setFunction(lua_State *state, const char *name, lua_CFunction function) {
  lua_pushcfunction(state, function, name);
  lua_setfield(state, -2, name);
}

} // namespace

int open(lua_State *state) {
  lua_newtable(state);
  setFunction(state, "read", read);
  setFunction(state, "readText", read);
  setFunction(state, "exists", exists);
  setFunction(state, "enumerateDirectory", enumerateDirectory);
  setFunction(state, "getCurrentDirectory", getCurrentDirectory);
  setFunction(state, "getAbsolutePath", getAbsolutePath);
  lua_setfield(state, LUA_REGISTRYINDEX,
               SINEN_LUA_PLUGIN_REGISTRY_PREFIX "nativefs");
  return 0;
}

int close(lua_State *state) {
  lua_pushnil(state);
  lua_setfield(state, LUA_REGISTRYINDEX,
               SINEN_LUA_PLUGIN_REGISTRY_PREFIX "nativefs");
  return 0;
}

} // namespace sinen::plugins::nativefs

SINEN_LUA_MODULE_EXPORT uint32_t sinen_lua_module_get_api_version(void) {
  return SINEN_LUA_MODULE_API_VERSION;
}

SINEN_LUA_MODULE_EXPORT int sinen_lua_module_open(lua_State *state) {
  return sinen::plugins::nativefs::open(state);
}

SINEN_LUA_MODULE_EXPORT int sinen_lua_module_close(lua_State *state) {
  return sinen::plugins::nativefs::close(state);
}
