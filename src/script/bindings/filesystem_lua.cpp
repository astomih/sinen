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
static auto lFilesystemExists(const char *path) {
  return Filesystem::exists(StringView(path));
}
static auto lFilesystemWrite(const char *path, const Buffer &buffer) {
  return Filesystem::write(StringView(path), buffer);
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
static auto lFilesystemGetUserDirectory() {
  auto path = Filesystem::getUserDirectory();
  return path;
}
void registerFilesystem(lua_State *L) {
  pushSnNamed(L, "Filesystem");
  luau::table(L, -1).raw_function("enumerateDirectory",
                                  lFilesystemEnumerateDirectory);
  luau::table(L, -1).raw_function("read", lFilesystemRead);
  luau::table(L, -1).raw_function("readText", lFilesystemReadText);
  luau::table(L, -1).function("exists", lFilesystemExists);
  luau::table(L, -1).function("write", lFilesystemWrite);
  luau::table(L, -1).raw_function("writeText", lFilesystemWriteText);
  luau::table(L, -1).function("getUserDirectory", lFilesystemGetUserDirectory);
  lua_pop(L, 1);
}
} // namespace sinen
