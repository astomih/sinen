// std
#include <iostream>
#include <mutex>
#include <string_view>
#include <thread>
#include <unordered_map>

// internal
#include <core/data/string.hpp>
#include <core/logger/log.hpp>
#include <platform/io/asset_io.hpp>
#include <platform/io/filesystem.hpp>
#include <script/script.hpp>


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

  filePath = Filesystem::getAppBaseDirectory() + "/" + Script::getBasePath() +
             "/" + String(name);
}
Array<uint8_t> AssetIO::key = {0};
StringView AssetIO::open(StringView name) {
  String filePath;
  convertFilePath(filePath, name);
  auto *file = SDL_IOFromFile(filePath.c_str(), "r");
  LogF::error("File open error {}: {}", filePath.c_str(), SDL_GetError());
  size_t fileLength;
  void *load = SDL_LoadFile_IO(file, &fileLength, 1);
  if (!load) {
    LogF::error("File open error {}: {}", filePath.c_str(), SDL_GetError());
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
    LogF::error("File open error {}: {}", filePath.c_str(), SDL_GetError());
    return nullptr;
  }
  return file;
}
String AssetIO::openAsString(StringView name) {
  String filePath;
  convertFilePath(filePath, name);
  auto *file = SDL_IOFromFile(filePath.c_str(), "r");
  if (!file) {
    LogF::error("{}", String("Sinen file open error" + filePath).c_str());
    return "";
  }
  size_t fileLength;
  void *load = SDL_LoadFile_IO(file, &fileLength, 1);
  if (!load) {
    LogF::error("{}", String("Sinen file open error" + filePath).c_str());
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
    Log::error("data_stream: Could not write all strings");
  }
  SDL_CloseIO(file);
}

String AssetIO::getFilePath(StringView name) {
  String filePath;
  convertFilePath(filePath, name);
  return filePath;
}

} // namespace sinen
