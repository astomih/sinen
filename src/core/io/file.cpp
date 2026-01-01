#include "core/allocator/global_allocator.hpp"
#include "core/buffer/buffer.hpp"
#include <SDL3/SDL.h>
#include <core/io/file.hpp>
#include <cstring>

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
