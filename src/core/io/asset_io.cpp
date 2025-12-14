// std
#include <iostream>
#include <mutex>
#include <string_view>
#include <thread>
#include <unordered_map>

// internal
#include "../../main_system.hpp"
#include <core/io/asset_io.hpp>
#include <core/io/file_system.hpp>
#include <core/logger/logger.hpp>

// external
#include <SDL3/SDL.h>
#include <SDL3/SDL_platform.h>

#ifdef __ANDROID__
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <jni.h>
#endif

namespace sinen {
static void convertFilePath(std::string &filePath, std::string_view name) {

  filePath = FileSystem::getAppBaseDirectory() + "/" +
             MainSystem::GetBasePath() + "/" + std::string(name);
}
std::vector<uint8_t> AssetIO::key = {0};
std::string_view AssetIO::open(std::string_view name) {
  std::string filePath;
  convertFilePath(filePath, name);
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
void *AssetIO::openAsIOStream(std::string_view name) {
  std::string filePath;
  convertFilePath(filePath, name);

  SDL_IOStream *file = SDL_IOFromFile(filePath.c_str(), "r");
  if (!file) {
    Logger::error("File open error %s", filePath.c_str());
    return nullptr;
  }
  return file;
}
std::string AssetIO::openAsString(std::string_view name) {
  std::string filePath;
  convertFilePath(filePath, name);
  auto *file = SDL_IOFromFile(filePath.c_str(), "r");
  if (!file) {
    Logger::error(std::string("Sinen file open error" + filePath).c_str());
    return "";
  }
  size_t fileLength;
  void *load = SDL_LoadFile_IO(file, &fileLength, 1);
  if (!load) {
    Logger::error(std::string("Sinen file open error" + filePath).c_str());
    return "";
  }
  std::string result{reinterpret_cast<char *>(load), fileLength};
  SDL_free(load);
  return result;
}

void AssetIO::write(std::string_view name, std::string_view data) {
  std::string filePath;
  convertFilePath(filePath, name);
  auto *file = SDL_IOFromFile(filePath.c_str(), "w");
  if (!file) {
    return;
  }
  if (data.size() != SDL_WriteIO(file, data.data(), data.size())) {
    Logger::error<>("data_stream: Could not write all strings");
  }
  SDL_CloseIO(file);
}

std::string AssetIO::getFilePath(std::string_view name) {
  std::string filePath;
  convertFilePath(filePath, name);
  return filePath;
}

} // namespace sinen
