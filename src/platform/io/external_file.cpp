#include <platform/io/external_file.hpp>

#include <core/allocator/engine_memory.hpp>
#include <core/logger/log.hpp>

#include <SDL3/SDL.h>

#include <cstring>
#include <filesystem>

namespace sinen {
namespace {
String pathPartToString(const std::filesystem::path &path) {
  const auto utf8 = path.u8string();
  return String(reinterpret_cast<const char *>(utf8.data()), utf8.size());
}
} // namespace

ExternalFile::ExternalFile(String path) : nativePath(std::move(path)) {}

std::optional<Buffer> ExternalFile::read() const {
  auto *file = SDL_IOFromFile(nativePath.c_str(), "rb");
  if (!file) {
    Log::error("External file open error {}: {}", name().c_str(),
               SDL_GetError());
    return std::nullopt;
  }

  size_t size = 0;
  void *loaded = SDL_LoadFile_IO(file, &size, true);
  if (!loaded) {
    Log::error("External file read error {}: {}", name().c_str(),
               SDL_GetError());
    return std::nullopt;
  }

  Buffer buffer = makeBuffer(size, BufferType::Binary, EngineMemory::asset());
  if (size > 0) {
    std::memcpy(buffer.data(), loaded, size);
  }
  SDL_free(loaded);
  return buffer;
}

String ExternalFile::name() const {
  return pathPartToString(
      std::filesystem::u8path(nativePath.c_str()).filename());
}

String ExternalFile::extension() const {
  return pathPartToString(
      std::filesystem::u8path(nativePath.c_str()).extension());
}

} // namespace sinen
