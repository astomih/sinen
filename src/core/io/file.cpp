// std libraries
#include <filesystem>
// Third party libraries
#include <SDL3/SDL.h>
// Local libraries
#include <core/io/file.hpp>

namespace sinen {
class File::impl {

public:
  impl() : rwops(nullptr) {}
  ~impl() = default;
  std::string open_mode_to_string(const Mode &mode) {
    std::string m;
    switch (mode) {
    case Mode::r:
      m = "r";
      break;
    case Mode::w:
      m = "w";
      break;
    case Mode::a:
      m = "a";
      break;
    case Mode::rp:
      m = "r+";
      break;
    case Mode::wp:
      m = "w+";
      break;
    case Mode::ap:
      m = "a+";
      break;
    case Mode::rb:
      m = "rb";
      break;
    case Mode::wb:
      m = "wb";
      break;
    case Mode::ab:
      m = "ab";
      break;
    case Mode::rpb:
      m = "r+b";
      break;
    case Mode::wpb:
      m = "w+b";
      break;
    case Mode::apb:
      m = "a+b";
      break;
    default:
      return "";
    }
    return m;
  }
  bool open(const char *filename, const Mode &mode) {
    return open(filename, open_mode_to_string(mode).c_str());
  };
  bool open(const char *filename, const char *mode) {
    rwops = SDL_IOFromFile(filename, mode);
    if (rwops == nullptr) {
      return false;
    }
    return true;
  };
  void close() {
    if (rwops) {
      SDL_CloseIO(rwops);
    }
  }
  void read(void *ptr, size_t size, size_t maxnum) {
    SDL_ReadIO(rwops, ptr, size);
  }
  void write(const void *ptr, size_t size, size_t num) {
    SDL_WriteIO(rwops, ptr, size);
  }
  void seek(long offset, SDL_IOWhence whence) {
    SDL_SeekIO(rwops, offset, whence);
  }
  std::int64_t tell() { return SDL_TellIO(rwops); }
  std::int64_t size() { return SDL_GetIOSize(rwops); }

private:
  SDL_IOStream *rwops;
};
File::File() : m_impl(new impl()) {}
File::~File() = default;
std::string File::OpenModeToString(const Mode &mode) {
  return m_impl->open_mode_to_string(mode);
}
bool File::Open(const char *filename, const Mode &mode) {
  return m_impl->open(filename, mode);
}
bool File::Open(std::string_view filename, const Mode &mode) {
  return m_impl->open(filename.data(), mode);
}
bool File::Open(const char *filename, const char *mode) {
  return m_impl->open(filename, mode);
}
bool File::Open(std::string_view filename, const char *mode) {
  return m_impl->open(filename.data(), mode);
}
void File::close() { m_impl->close(); }

void File::Read(void *ptr, size_t size, size_t maxnum) {
  m_impl->read(ptr, size, maxnum);
}
void File::Write(const void *buffer, size_t size, size_t num) {
  m_impl->write(buffer, size, num);
}
void File::Seek(const std::int64_t &offset, int whence) {
  m_impl->seek(offset, static_cast<SDL_IOWhence>(whence));
}
std::int64_t File::Tell() { return m_impl->tell(); }
std::int64_t File::Size() { return m_impl->size(); }
} // namespace sinen
