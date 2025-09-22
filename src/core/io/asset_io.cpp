// std
#include <iostream>
#include <mutex>
#include <string_view>
#include <thread>
#include <unordered_map>
// internal
#include <core/io/asset_io.hpp>
#include <core/logger/logger.hpp>
// external
#include "../../main_system.hpp"

#include <SDL3/SDL.h>

namespace sinen {
std::vector<uint8_t> AssetIO::key = {0};
std::string_view AssetIO::Open(const AssetType &type, std::string_view name) {
  std::string filePath;
  ConvertFilePath(type, filePath, name);

  auto *file = SDL_IOFromFile(filePath.c_str(), "r");
  Logger::Error("File open error %s: %s", filePath.c_str(), SDL_GetError());
  size_t fileLength;
  void *load = SDL_LoadFile_IO(file, &fileLength, 1);
  if (!load) {
    Logger::Error("File load error %s: %s", filePath.c_str(), SDL_GetError());
    return "";
  }
  std::string_view result(reinterpret_cast<char *>(load), fileLength);
  SDL_free(load);
  return result;
}
void *AssetIO::OpenAsRWOps(const AssetType &type, std::string_view name) {
  std::string filePath;
  ConvertFilePath(type, filePath, name);

  SDL_IOStream *file = SDL_IOFromFile(filePath.c_str(), "r");
  if (!file) {
    Logger::Error("File open error %s", filePath.c_str());
    return nullptr;
  }
  return file;
}
std::string AssetIO::OpenAsString(const AssetType &type,
                                  std::string_view name) {
  std::string filePath;
  ConvertFilePath(type, filePath, name);

  auto *file = SDL_IOFromFile(filePath.c_str(), "r");
  if (!file) {
    return "";
  }
  size_t fileLength;
  void *load = SDL_LoadFile_IO(file, &fileLength, 1);
  if (!load) {
    return "";
  }
  std::string result{reinterpret_cast<char *>(load), fileLength};
  SDL_free(load);
  return result;
}

void AssetIO::Write(const AssetType &type, std::string_view name,
                    std::string_view data) {
  std::string filePath;
  ConvertFilePath(type, filePath, name);
  auto *file = SDL_IOFromFile(filePath.c_str(), "w");
  if (!file) {
    return;
  }
  if (data.size() != SDL_WriteIO(file, data.data(), data.size())) {
    Logger::Error<>("data_stream: Could not write all strings");
  }
  SDL_CloseIO(file);
}
void AssetIO::ConvertFilePath(const AssetType &type, std::string &filePath,
                              std::string_view name) {
  std::string base = MainSystem::GetBasePath() + "/asset/";
  switch (type) {
  case AssetType::Font:
    filePath += base + std::string{"font/"} + name.data();
    break;

  case AssetType::Model:
    filePath += base + std::string{"model/"} + name.data();
    break;
  case AssetType::Music:
    filePath = base + std::string{"music/"} + name.data();
    break;
  case AssetType::Script:
    filePath = base + std::string{"script/"} + name.data();
    break;
  case AssetType::Shader:
    filePath = base + std::string{"shader/"} + name.data();
    break;
  case AssetType::Sound:
    filePath = base + std::string{"sound/"} + name.data();
    break;
  case AssetType::Texture:
    filePath = base + std::string{"texture/"} + name.data();
    break;

  default:
    break;
  }
}
std::string AssetIO::ConvertFilePath(const AssetType &type,
                                     std::string_view name) {
  std::string filePath;
  ConvertFilePath(type, filePath, name);
  return filePath;
}

} // namespace sinen
