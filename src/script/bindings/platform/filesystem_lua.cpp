#include <script/luaapi.hpp>
#include <platform/io/asset_io.hpp>
#include <platform/io/filesystem.hpp>

namespace sinen {
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
static int lFilesystemRead(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  auto buffer = Filesystem::read(StringView(path));
  if (!buffer) {
    lua_pushnil(L);
    return 1;
  }
  udNewOwned<Buffer>(L, std::move(*buffer));
  return 1;
}
static int lFilesystemExists(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  lua_pushboolean(L, Filesystem::exists(StringView(path)));
  return 1;
}
static int lFilesystemWrite(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const auto &buffer = udValue<Buffer>(L, 2);
  lua_pushboolean(L, Filesystem::write(StringView(path), buffer));
  return 1;
}
static int lFilesystemGetUserDirectory(lua_State *L) {
  auto path = Filesystem::getUserDirectory();
  lua_pushlstring(L, path.data(), path.size());
  return 1;
}
void registerFilesystem(lua_State *L) {
  pushSnNamed(L, "Filesystem");
  luaPushcfunction2(L, lFilesystemEnumerateDirectory);
  lua_setfield(L, -2, "enumerateDirectory");
  luaPushcfunction2(L, lFilesystemRead);
  lua_setfield(L, -2, "read");
  luaPushcfunction2(L, lFilesystemExists);
  lua_setfield(L, -2, "exists");
  luaPushcfunction2(L, lFilesystemWrite);
  lua_setfield(L, -2, "write");
  luaPushcfunction2(L, lFilesystemGetUserDirectory);
  lua_setfield(L, -2, "getUserDirectory");
  lua_pop(L, 1);
}
} // namespace sinen
