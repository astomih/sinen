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
  if (!file)
    return std::string("");
  size_t fileLength;
  void *load = SDL_LoadFile_RW(file, &fileLength, 1);
  if (!load)
    return std::string("");
  std::string result{reinterpret_cast<char *>(load), fileLength};
  SDL_free(load);
  return result;
}
void asset_reader::ConvertFilePath(std::string &filePath, std::string_view name,
                                   const asset_type &assetType) {

  switch (assetType) {
  case asset_type::Effect:
    filePath += std::string{"Assets/Effect/"} + name.data();
    break;
  case asset_type::Font:
    filePath += std::string{"Assets/Font/"} + name.data();
    break;

  case asset_type::Model:
    filePath += std::string{"Assets/Model/"} + name.data();
    break;
  case asset_type::Music:
    filePath = std::string{"Assets/Music/"} + name.data();
    break;
  case asset_type::Script:
    filePath = std::string{"Assets/Script/"} + name.data();
    break;
  case asset_type::Shader:
    filePath = std::string{"Assets/Shader/"} + name.data();
    break;
  case asset_type::Sound:
    filePath = std::string{"Assets/Sound/"} + name.data();
    break;
  case asset_type::Texture:
    filePath = std::string{"Assets/Texture/"} + name.data();
    break;

  default:
    break;
  }
}
} // namespace nen