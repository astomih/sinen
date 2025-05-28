// std
#include <iostream>
#include <mutex>
#include <string_view>
#include <thread>
#include <unordered_map>
// internal
#include <core/io/data_stream.hpp>
#include <core/logger/logger.hpp>
// external
#include <SDL3/SDL.h>

namespace sinen {
void decoding(std::string &str, std::vector<uint8_t> &key) {
  int i = 0;
  for (auto &c : str) {
    uint8_t uc = (uint8_t)c;
    uc ^= (uint8_t)key[i % key.size()];
    c = (char)uc;
    i++;
  }
}
std::vector<uint8_t> DataStream::key = {0};
std::string_view DataStream::open(const AssetType &type,
                                  std::string_view name) {
  std::string filePath;
  convert_file_path(type, filePath, name);

  auto *file = SDL_IOFromFile(filePath.c_str(), "r");
  Logger::error("File open error %s: %s", filePath.c_str(), SDL_GetError());
  size_t fileLength;
  void *load = SDL_LoadFile_IO(file, &fileLength, 1);
  if (!load) {
    Logger::error("File load error %s: %s", filePath.c_str(), SDL_GetError());
    return "";
  }
  std::string_view result(reinterpret_cast<char *>(load), fileLength);
  SDL_free(load);
  return result;
}
void *DataStream::open_as_rwops(const AssetType &type, std::string_view name) {
  std::string filePath;
  convert_file_path(type, filePath, name);

  SDL_IOStream *file = SDL_IOFromFile(filePath.c_str(), "r");
  if (!file) {
    Logger::error("File open error %s", filePath.c_str());
    return nullptr;
  }
  return file;
}
std::string DataStream::open_as_string(const AssetType &type,
                                       std::string_view name) {
  std::string filePath;
  convert_file_path(type, filePath, name);

  auto *file = SDL_IOFromFile(filePath.c_str(), "r");
  if (!file) {
    Logger::warn("File not found %s: %s", filePath.c_str(), SDL_GetError());
    return "";
  }
  size_t fileLength;
  void *load = SDL_LoadFile_IO(file, &fileLength, 1);
  if (!load) {
    Logger::error("File load error %s: %s", filePath.c_str(), SDL_GetError());
    return "";
  }
  std::string result{reinterpret_cast<char *>(load), fileLength};
  SDL_free(load);
  if (name.ends_with(".sia"))
    decoding(result, key);
  return result;
}

void DataStream::write(const AssetType &type, std::string_view name,
                       std::string_view data) {
  std::string filePath;
  convert_file_path(type, filePath, name);
  auto *file = SDL_IOFromFile(filePath.c_str(), "w");
  if (!file) {
    return;
  }
  if (data.size() != SDL_WriteIO(file, data.data(), data.size())) {
    Logger::error<>("data_stream: Could not write all strings");
  }
  SDL_CloseIO(file);
}
void DataStream::convert_file_path(const AssetType &type, std::string &filePath,
                                   std::string_view name) {
  std::string base = "data/";
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
    filePath = std::string{"script/"} + name.data();
    break;
  case AssetType::Shader:
    filePath = base + std::string{"shader/"} + name.data();
    break;
  case AssetType::Scene:
    filePath = base + std::string{"scene/"} + name.data();
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
std::string DataStream::convert_file_path(const AssetType &type,
                                          std::string_view name) {
  std::string filePath;
  convert_file_path(type, filePath, name);
  return filePath;
}

} // namespace sinen
