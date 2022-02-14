#include <Nen.hpp>
#include <SDL.h>
#include <SDL_rwops.h>
#include <mutex>
#include <thread>

namespace nen {

std::string_view asset_reader::Load(const asset_type &assetType,
                                    std::string_view name) {
  std::string filePath;
  ConvertFilePath(filePath, name, assetType);

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
void *asset_reader::LoadAsRWops(const asset_type &assetType,
                                std::string_view name) {
  std::string filePath;
  ConvertFilePath(filePath, name, assetType);

  SDL_RWops *file{SDL_RWFromFile(filePath.c_str(), "r")};
#ifndef NEN_NO_EXCEPTION
  if (!file)
    throw std::runtime_error("File open error.");
#endif
  return file;
}
std::string asset_reader::LoadAsString(const asset_type &assetType,
                                       std::string_view name) {
  std::string filePath;
  ConvertFilePath(filePath, name, assetType);

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
  return result;
}
void asset_reader::ConvertFilePath(std::string &filePath, std::string_view name,
                                   const asset_type &assetType) {

  switch (assetType) {
  case asset_type::Font:
    filePath += std::string{"data/font/"} + name.data();
    break;

  case asset_type::Model:
    filePath += std::string{"data/model/"} + name.data();
    break;
  case asset_type::Music:
    filePath = std::string{"data/music/"} + name.data();
    break;
  case asset_type::Script:
    filePath = std::string{"data/script/"} + name.data();
    break;
  case asset_type::gl_shader:
    filePath = std::string{"data/shader/GL/"} + name.data();
    break;
  case asset_type::vk_shader:
    filePath = std::string{"data/shader/Vulkan/"} + name.data();
    break;
  case asset_type::Sound:
    filePath = std::string{"data/sound/"} + name.data();
    break;
  case asset_type::Texture:
    filePath = std::string{"data/texture/"} + name.data();
    break;

  default:
    break;
  }
}
} // namespace nen