#include <SDL.h>
#include <SDL_rwops.h>
#include <io/dstream.hpp>
#include <iostream>
#include <mutex>
#include <string_view>
#include <thread>
#include <unordered_map>

namespace nen {
void decoding(std::string &str, std::vector<uint8_t> &key) {
  int i = 0;
  for (auto &c : str) {
    uint8_t uc = (uint8_t)c;
    uc ^= (uint8_t)key[i % key.size()];
    c = (char)uc;
    i++;
  }
}
std::vector<uint8_t> dstream::key = {0};
std::string_view dstream::open(const asset_type &assetType,
                               std::string_view name) {
  std::string filePath;
  convert_file_path(filePath, name, assetType);

  SDL_RWops *file = SDL_RWFromFile(filePath.c_str(), "r");
#ifndef NEN_NO_EXCEPTION
  if (!file)
    throw std::runtime_error("File open error.");
#else
  std::cerr << "File open error" << std::endl;
#endif
  size_t fileLength;
  void *load = SDL_LoadFile_RW(file, &fileLength, 1);
#ifndef NEN_NO_EXCEPTION
  if (!load)
    throw std::runtime_error("convert error.");
#endif
  std::string_view result(reinterpret_cast<char *>(load), fileLength);
  SDL_free(load);
  return result;
}
static std::unordered_map<std::string, std::string> map;
void *dstream::open_as_rwops(const asset_type &assetType,
                             std::string_view name) {
  std::string filePath;
  convert_file_path(filePath, name, assetType);

  map[name.data()] = open_as_string(assetType, name);

  SDL_RWops *file =
      SDL_RWFromConstMem(map[name.data()].data(), map[name.data()].size());
#ifndef NEN_NO_EXCEPTION
  if (!file)
    throw std::runtime_error("File open error.");
#endif
  return file;
}
std::string dstream::open_as_string(const asset_type &assetType,
                                    std::string_view name) {
  std::string filePath;
  convert_file_path(filePath, name, assetType);

  SDL_RWops *file = SDL_RWFromFile(filePath.c_str(), "r");
  if (!file) {
    return std::string("");
  }
  size_t fileLength;
  void *load = SDL_LoadFile_RW(file, &fileLength, 1);
  if (!load) {
    return std::string("");
  }
  std::string result{reinterpret_cast<char *>(load), fileLength};
  SDL_free(load);
  if (name.ends_with(".sia"))
    decoding(result, key);
  return result;
}

void dstream::write(const asset_type &assetType, std::string_view name,
                    std::string_view data) {
  std::string filePath;
  convert_file_path(filePath, name, assetType);
  SDL_RWops *file = SDL_RWFromFile(filePath.c_str(), "w");
  if (!file) {
    return;
  }
  SDL_RWwrite(file, data.data(), 1, data.size());
}
void dstream::convert_file_path(std::string &filePath, std::string_view name,
                                const asset_type &assetType) {
  std::string base = "data/";
  switch (assetType) {
  case asset_type::Font:
    filePath += base + std::string{"font/"} + name.data();
    break;

  case asset_type::Model:
    filePath += base + std::string{"model/"} + name.data();
    break;
  case asset_type::Music:
    filePath = base + std::string{"music/"} + name.data();
    break;
  case asset_type::Script:
    filePath = base + std::string{"script/"} + name.data();
    break;
  case asset_type::gl_shader:
    filePath = base + std::string{"shader/GL/"} + name.data();
    break;
  case asset_type::vk_shader:
    filePath = base + std::string{"shader/Vulkan/"} + name.data();
    break;
  case asset_type::Sound:
    filePath = base + std::string{"sound/"} + name.data();
    break;
  case asset_type::Texture:
    filePath = base + std::string{"texture/"} + name.data();
    break;

  default:
    break;
  }
}
std::string dstream::convert_file_path(std::string_view name,
                                       const asset_type &assetType) {
  std::string filePath;
  convert_file_path(filePath, name, assetType);
  return filePath;
}

} // namespace nen