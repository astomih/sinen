// std libraries
#include <filesystem>
// Third party libraries
#include <SDL2/SDL.h>
// Local libraries
#include <io/file.hpp>

namespace sinen {
class file::impl {

public:
  impl() : rwops(nullptr) {}
  ~impl() = default;
  bool open(const char *filename, const open_mode &mode) {
    switch (mode) {
    case open_mode::r:
      rwops = SDL_RWFromFile(filename, "r");
      break;
    case open_mode::w:
      rwops = SDL_RWFromFile(filename, "w");
      break;
    case open_mode::a:
      rwops = SDL_RWFromFile(filename, "a");
      break;
    case open_mode::rw:
      rwops = SDL_RWFromFile(filename, "r+");
      break;
    case open_mode::wa:
      rwops = SDL_RWFromFile(filename, "w+");
      break;
    case open_mode::rwa:
      rwops = SDL_RWFromFile(filename, "a+");
      break;
    case open_mode::rb:
      rwops = SDL_RWFromFile(filename, "rb");
      break;
    case open_mode::wb:
      rwops = SDL_RWFromFile(filename, "wb");
      break;
    case open_mode::ab:
      rwops = SDL_RWFromFile(filename, "ab");
      break;
    case open_mode::rwb:
      rwops = SDL_RWFromFile(filename, "r+b");
      break;
    case open_mode::wab:
      rwops = SDL_RWFromFile(filename, "w+b");
      break;
    case open_mode::rwab:
      rwops = SDL_RWFromFile(filename, "a+b");
      break;
    default:
      return false;
    }
    if (rwops == nullptr) {
      return false;
    }
    return true;
  };
  void close() {
    if (rwops) {
      SDL_RWclose(rwops);
    }
  }
  void read(void *ptr, size_t size, size_t maxnum) {
    SDL_RWread(rwops, ptr, size, maxnum);
  }
  void write(const void *ptr, size_t size, size_t num) {
    SDL_RWwrite(rwops, ptr, size, num);
  }
  void seek(long offset, int whence) { SDL_RWseek(rwops, offset, whence); }
  std::int64_t tell() { return SDL_RWtell(rwops); }
  std::int64_t size() { return SDL_RWsize(rwops); }

private:
  SDL_RWops *rwops;
};
file::file() : m_impl(new impl()) {}
file::~file() = default;
bool file::open(const char *filename, const open_mode &mode) {
  return m_impl->open(filename, mode);
}
bool file::open(std::string_view filename, const open_mode &mode) {
  return m_impl->open(filename.data(), mode);
}
void file::close() { m_impl->close(); }

void file::read(void *ptr, size_t size, size_t maxnum) {
  m_impl->read(ptr, size, maxnum);
}
void file::write(const void *buffer, size_t size, size_t num) {
  m_impl->write(buffer, size, num);
}
void file::seek(const std::int64_t &offset, int whence) {
  m_impl->seek(offset, whence);
}
std::int64_t file::tell() { return m_impl->tell(); }
std::int64_t file::size() { return m_impl->size(); }
} // namespace sinen
