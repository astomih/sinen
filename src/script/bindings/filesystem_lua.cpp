#include "luaapi.hpp"
#include <core/buffer/buffer.hpp>
#include <platform/io/asset_reader.hpp>
#include <platform/io/filesystem.hpp>

#include <cstring>

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
static int lFilesystemReadText(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  auto buffer = Filesystem::read(StringView(path));
  if (!buffer) {
    lua_pushnil(L);
    return 1;
  }
  lua_pushlstring(L, static_cast<const char *>(buffer->data()),
                  static_cast<size_t>(buffer->size()));
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
static int lFilesystemWriteText(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  size_t textSize = 0;
  const char *text = luaL_checklstring(L, 2, &textSize);
  Buffer buffer = makeBuffer(textSize, BufferType::String);
  if (textSize > 0) {
    std::memcpy(buffer.data(), text, textSize);
  }
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
  Binding::registerFunction(L, "enumerateDirectory",
                            lFilesystemEnumerateDirectory);
  Binding::registerFunction(L, "read", lFilesystemRead);
  Binding::registerFunction(L, "readText", lFilesystemReadText);
  Binding::registerFunction(L, "exists", lFilesystemExists);
  Binding::registerFunction(L, "write", lFilesystemWrite);
  Binding::registerFunction(L, "writeText", lFilesystemWriteText);
  Binding::registerFunction(L, "getUserDirectory", lFilesystemGetUserDirectory);
  lua_pop(L, 1);
}
} // namespace sinen
