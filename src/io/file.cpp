// std libraries
#include <filesystem>
// Third party libraries
#include <SDL.h>
// Local libraries
#include <io/file.hpp>

namespace sinen {
class File::impl {

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
File::File() : m_impl(new impl()) {}
File::~File() = default;
std::string File::open_mode_to_string(const mode &mode) {
  return m_impl->open_mode_to_string(mode);
}
bool File::open(const char *filename, const mode &mode) {
  return m_impl->open(filename, mode);
}
bool File::open(std::string_view filename, const mode &mode) {
  return m_impl->open(filename.data(), mode);
}
bool File::open(const char *filename, const char *mode) {
  return m_impl->open(filename, mode);
}
bool File::open(std::string_view filename, const char *mode) {
  return m_impl->open(filename.data(), mode);
}
void File::close() { m_impl->close(); }

void File::read(void *ptr, size_t size, size_t maxnum) {
  m_impl->read(ptr, size, maxnum);
}
void File::write(const void *buffer, size_t size, size_t num) {
  m_impl->write(buffer, size, num);
}
void File::seek(const std::int64_t &offset, int whence) {
  m_impl->seek(offset, whence);
}
std::int64_t File::tell() { return m_impl->tell(); }
std::int64_t File::size() { return m_impl->size(); }
} // namespace sinen
