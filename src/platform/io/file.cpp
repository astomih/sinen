#include "core/allocator/global_allocator.hpp"
#include "core/buffer/buffer.hpp"
#include <SDL3/SDL.h>
#include <cstring>
#include <platform/io/file.hpp>

namespace sinen {
File::File() : stream(nullptr) {}
File::~File() { close(); }
bool File::open(StringView filename, StringView mode) {
  stream = (void *)SDL_IOFromFile(filename.data(), mode.data());
  if (stream == nullptr) {
    return false;
  }
  return true;
}
void File::close() {
  if (stream) {
    SDL_CloseIO((SDL_IOStream *)stream);
    stream = nullptr;
  }
}

Buffer File::read(size_t size) {
  void *ptr = GlobalAllocator::get()->allocate(size);
  memset(ptr, 0, size);
  auto read = SDL_ReadIO((SDL_IOStream *)stream, ptr, size);
  if (read < size) {
    memset((std::byte *)ptr + read, 0, size - read);
  }
  return makeBuffer(ptr, size);
}
void File::write(const Buffer &buffer) {
  SDL_WriteIO((SDL_IOStream *)stream, buffer.data(), buffer.size());
}

void File::seek(const std::int64_t &offset) {
  SDL_SeekIO((SDL_IOStream *)stream, offset, SDL_IO_SEEK_SET);
}
std::int64_t File::tell() { return SDL_TellIO((SDL_IOStream *)stream); }
std::int64_t File::size() { return SDL_GetIOSize((SDL_IOStream *)stream); }
} // namespace sinen

#include <script/binding.hpp>
namespace sinen {
void registerFile(lua_State *L) {
  Binding binding(L);
  binding.beginClass(File::metaTableName(), udPtrGc<File>, "File",
                     [](lua_State *L) {
                       udPushPtr<File>(L, makePtr<File>());
                       return 1;
                     });
  binding.registerFunction("open", [](lua_State *L) {
    auto &f = udPtr<File>(L, 1);
    const char *filename = luaL_checkstring(L, 2);
    const char *mode = luaL_checkstring(L, 3);
    f->open(StringView(filename), StringView(mode));
    return 0;
  });
  binding.registerFunction("close", [](lua_State *L) {
    auto &f = udPtr<File>(L, 1);
    f->close();
    return 0;
  });
  binding.registerFunction("read", [](lua_State *L) {
    auto &f = udPtr<File>(L, 1);
    size_t size = static_cast<size_t>(luaL_checkinteger(L, 2));
    Buffer buf = f->read(size);
    udNewOwned<Buffer>(L, std::move(buf));
    return 1;
  });
  binding.registerFunction("write", [](lua_State *L) {
    auto &f = udPtr<File>(L, 1);
    auto &buf = udValue<Buffer>(L, 2);
    f->write(buf);
    return 0;
  });
  binding.registerFunction("seek", [](lua_State *L) {
    auto &f = udPtr<File>(L, 1);
    std::int64_t offset = static_cast<std::int64_t>(luaL_checkinteger(L, 2));
    f->seek(offset);
    return 0;
  });
  binding.registerFunction("tell", [](lua_State *L) {
    auto &f = udPtr<File>(L, 1);
    std::int64_t pos = f->tell();
    lua_pushinteger(L, pos);
    return 1;
  });
  binding.registerFunction("size", [](lua_State *L) {
    auto &f = udPtr<File>(L, 1);
    std::int64_t size = f->size();
    lua_pushinteger(L, size);
    return 1;
  });
  binding.endClass();
}
} // namespace sinen
