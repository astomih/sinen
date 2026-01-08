// std
#include <iostream>
#include <mutex>
#include <string_view>
#include <thread>
#include <unordered_map>

// internal
#include <asset/script/script.hpp>
#include <core/data/string.hpp>
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
static void convertFilePath(String &filePath, StringView name) {

  filePath = FileSystem::getAppBaseDirectory() + "/" + Script::getBasePath() +
             "/" + String(name);
}
Array<uint8_t> AssetIO::key = {0};
StringView AssetIO::open(StringView name) {
  String filePath;
  convertFilePath(filePath, name);
  auto *file = SDL_IOFromFile(filePath.c_str(), "r");
  Logger::error("File open error %s: %s", filePath.c_str(), SDL_GetError());
  size_t fileLength;
  void *load = SDL_LoadFile_IO(file, &fileLength, 1);
  if (!load) {
    Logger::error("File load error %s: %s", filePath.c_str(), SDL_GetError());
    return "";
  }
  StringView result(reinterpret_cast<char *>(load), fileLength);
  SDL_free(load);
  return result;
}
void *AssetIO::openAsIOStream(StringView name) {
  String filePath;
  convertFilePath(filePath, name);

  SDL_IOStream *file = SDL_IOFromFile(filePath.c_str(), "r");
  if (!file) {
    Logger::error("File open error %s", filePath.c_str());
    return nullptr;
  }
  return file;
}
String AssetIO::openAsString(StringView name) {
  String filePath;
  convertFilePath(filePath, name);
  auto *file = SDL_IOFromFile(filePath.c_str(), "r");
  if (!file) {
    Logger::error("%s", String("Sinen file open error" + filePath).c_str());
    return "";
  }
  size_t fileLength;
  void *load = SDL_LoadFile_IO(file, &fileLength, 1);
  if (!load) {
    Logger::error("%s", String("Sinen file open error" + filePath).c_str());
    return "";
  }
  String result{reinterpret_cast<char *>(load), fileLength};
  SDL_free(load);
  return result;
}

void AssetIO::write(StringView name, StringView data) {
  String filePath;
  convertFilePath(filePath, name);
  auto *file = SDL_IOFromFile(filePath.c_str(), "w");
  if (!file) {
    return;
  }
  if (data.size() != SDL_WriteIO(file, data.data(), data.size())) {
    Logger::error("data_stream: Could not write all strings");
  }
  SDL_CloseIO(file);
}

String AssetIO::getFilePath(StringView name) {
  String filePath;
  convertFilePath(filePath, name);
  return filePath;
}

} // namespace sinen
