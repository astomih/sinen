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
  std::string open_mode_to_string(const mode &mode) {
    std::string m;
    switch (mode) {
    case mode::r:
      m = "r";
      break;
    case mode::w:
      m = "w";
      break;
    case mode::a:
      m = "a";
      break;
    case mode::rp:
      m = "r+";
      break;
    case mode::wp:
      m = "w+";
      break;
    case mode::ap:
      m = "a+";
      break;
    case mode::rb:
      m = "rb";
      break;
    case mode::wb:
      m = "wb";
      break;
    case mode::ab:
      m = "ab";
      break;
    case mode::rpb:
      m = "r+b";
      break;
    case mode::wpb:
      m = "w+b";
      break;
    case mode::apb:
      m = "a+b";
      break;
    default:
      return "";
    }
    return m;
  }
  bool open(const char *filename, const mode &mode) {
    return open(filename, open_mode_to_string(mode).c_str());
  };
  bool open(const char *filename, const char *mode) {
    rwops = SDL_RWFromFile(filename, mode);
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
std::string file::open_mode_to_string(const mode &mode) {
  return m_impl->open_mode_to_string(mode);
}
bool file::open(const char *filename, const mode &mode) {
  return m_impl->open(filename, mode);
}
bool file::open(std::string_view filename, const mode &mode) {
  return m_impl->open(filename.data(), mode);
}
bool file::open(const char *filename, const char *mode) {
  return m_impl->open(filename, mode);
}
bool file::open(std::string_view filename, const char *mode) {
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
