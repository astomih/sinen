// std
#include <algorithm>
#include <cctype>
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
#include <miniz.h>

#ifdef __ANDROID__
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <jni.h>
#endif

namespace sinen {
namespace {
std::mutex gArchiveMutex;
mz_zip_archive gArchive{};
bool gArchiveMounted = false;
String gArchivePath;

static String normalizePath(StringView path) {
  Array<String> parts;
  String current;
  for (char c : path) {
    const char normalized = (c == '\\') ? '/' : c;
    if (normalized == '/') {
      if (current.empty() || current == ".") {
        current.clear();
      } else if (current == "..") {
        if (!parts.empty()) {
          parts.pop_back();
        }
        current.clear();
      } else {
        parts.push_back(current);
        current.clear();
      }
      continue;
    }
    current.push_back(normalized);
  }
  if (!current.empty() && current != ".") {
    if (current == "..") {
      if (!parts.empty()) {
        parts.pop_back();
      }
    } else {
      parts.push_back(current);
    }
  }

  String out;
  for (size_t i = 0; i < parts.size(); ++i) {
    if (i > 0) {
      out.push_back('/');
    }
    out += parts[i];
  }
  return out;
}

static bool endsWithIcaseAscii(StringView value, StringView suffix) {
  if (suffix.size() > value.size()) {
    return false;
  }
  const size_t offset = value.size() - suffix.size();
  for (size_t i = 0; i < suffix.size(); ++i) {
    const auto a = static_cast<unsigned char>(value[offset + i]);
    const auto b = static_cast<unsigned char>(suffix[i]);
    if (std::tolower(a) != std::tolower(b)) {
      return false;
    }
  }
  return true;
}

static String archiveEntryPath(StringView name) {
  String logicalPath;
  if (name.starts_with("user://")) {
    return "";
  }
  if (!name.empty() && (name[0] == '/' || name[0] == '\\')) {
    logicalPath = Script::getRootBasePath() + "/" + String(name.substr(1));
  } else {
    logicalPath = Script::getBasePath() + "/" + String(name);
  }
  String normalized = normalizePath(logicalPath);
  if (normalized.empty()) {
    return AssetIO::archiveRootDirectory();
  }
  return String(AssetIO::archiveRootDirectory()) + "/" + normalized;
}

static String normalizeArchivePath(StringView path) {
  String normalized = normalizePath(path);
  if (normalized == AssetIO::archiveRootDirectory() ||
      normalized.starts_with(String(AssetIO::archiveRootDirectory()) + "/")) {
    return normalized;
  }
  if (normalized.empty()) {
    return AssetIO::archiveRootDirectory();
  }
  return String(AssetIO::archiveRootDirectory()) + "/" + normalized;
}

static bool archiveEntryExistsLocked(StringView path) {
  if (!gArchiveMounted) {
    return false;
  }
  const String normalized = normalizeArchivePath(path);
  return mz_zip_reader_locate_file(&gArchive, normalized.c_str(), nullptr, 0) >=
         0;
}

static bool archiveDirectoryExistsLocked(StringView path) {
  if (!gArchiveMounted) {
    return false;
  }
  String normalized = normalizeArchivePath(path);
  if (!normalized.empty() && normalized.back() != '/') {
    normalized.push_back('/');
  }
  const mz_uint count = mz_zip_reader_get_num_files(&gArchive);
  for (mz_uint i = 0; i < count; ++i) {
    mz_zip_archive_file_stat stat{};
    if (!mz_zip_reader_file_stat(&gArchive, i, &stat)) {
      continue;
    }
    String name = normalizePath(stat.m_filename);
    if (!name.empty() && name.back() != '/' &&
        mz_zip_reader_is_file_a_directory(&gArchive, i)) {
      name.push_back('/');
    }
    if (name.starts_with(normalized)) {
      return true;
    }
  }
  return false;
}

static String openArchiveEntryAsStringLocked(StringView path) {
  if (!gArchiveMounted) {
    return "";
  }
  const String normalized = normalizeArchivePath(path);
  const int index =
      mz_zip_reader_locate_file(&gArchive, normalized.c_str(), nullptr, 0);
  if (index < 0) {
    return "";
  }

  size_t size = 0;
  void *data = mz_zip_reader_extract_to_heap(
      &gArchive, static_cast<mz_uint>(index), &size, 0);
  if (!data) {
    Log::error("Sinen archive read error {}: {}", normalized.c_str(),
               mz_zip_get_error_string(mz_zip_get_last_error(&gArchive)));
    return "";
  }

  String result(reinterpret_cast<const char *>(data), size);
  mz_free(data);
  return result;
}
} // namespace

static bool convertFilePath(String &filePath, StringView name,
                            FilesystemAccess access) {
  if (!Filesystem::resolveSandboxPath(name, access, filePath)) {
    Log::error("Filesystem sandbox rejected asset path: {}",
               String(name).c_str());
    filePath.clear();
    return false;
  }
  return true;
}
Array<uint8_t> AssetIO::key = {0};
StringView AssetIO::open(StringView name) {
  static thread_local String result;
  result = openAsString(name);
  return result;
}
void *AssetIO::openAsIOStream(StringView name) {
  String filePath;
  if (!convertFilePath(filePath, name, FilesystemAccess::Read)) {
    return nullptr;
  }

  SDL_IOStream *file = SDL_IOFromFile(filePath.c_str(), "rb");
  if (!file) {
    Log::error("Asset open error {}: {}", filePath.c_str(), SDL_GetError());
    return nullptr;
  }
  return file;
}
String AssetIO::openAsString(StringView name) {
  if (!name.starts_with("user://") && isArchiveMounted()) {
    const String path = archiveEntryPath(name);
    if (!path.empty() && archiveEntryExists(path)) {
      return openArchiveEntryAsString(path);
    }
  }

  String filePath;
  if (!convertFilePath(filePath, name, FilesystemAccess::Read)) {
    return "";
  }
  auto *file = SDL_IOFromFile(filePath.c_str(), "rb");
  if (!file) {
    Log::error("Sinen file open error {}: {}", filePath.c_str(),
               SDL_GetError());
    return "";
  }
  size_t fileLength;
  void *load = SDL_LoadFile_IO(file, &fileLength, 1);
  if (!load) {
    Log::error("Sinen file read error {}: {}", filePath.c_str(),
               SDL_GetError());
    return "";
  }
  String result{reinterpret_cast<char *>(load), fileLength};
  SDL_free(load);
  return result;
}

void AssetIO::write(StringView name, StringView data) {
  String filePath;
  if (!convertFilePath(filePath, name, FilesystemAccess::Write)) {
    return;
  }
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
  convertFilePath(filePath, name, FilesystemAccess::Read);
  return filePath;
}

String AssetIO::getLoadPath(StringView name) {
  if (!name.starts_with("user://") && isArchiveMounted()) {
    return archiveEntryPath(name);
  }
  return getFilePath(name);
}

bool AssetIO::mountArchive(StringView path) {
  if (!endsWithIcaseAscii(path, archiveExtension())) {
    Log::error("Unsupported archive extension: {}", String(path).c_str());
    return false;
  }

  std::lock_guard<std::mutex> lock(gArchiveMutex);
  if (gArchiveMounted) {
    mz_zip_reader_end(&gArchive);
    gArchiveMounted = false;
    gArchivePath.clear();
    mz_zip_zero_struct(&gArchive);
  }

  mz_zip_zero_struct(&gArchive);
  if (!mz_zip_reader_init_file(&gArchive, String(path).c_str(), 0)) {
    Log::error("Failed to open Sinen archive {}: {}", String(path).c_str(),
               mz_zip_get_error_string(mz_zip_get_last_error(&gArchive)));
    mz_zip_zero_struct(&gArchive);
    return false;
  }

  gArchiveMounted = true;
  gArchivePath = path;
  if (!archiveDirectoryExistsLocked(archiveRootDirectory())) {
    Log::error("Sinen archive {} must contain a {} directory at the zip root",
               gArchivePath.c_str(), archiveRootDirectory());
    mz_zip_reader_end(&gArchive);
    gArchiveMounted = false;
    gArchivePath.clear();
    mz_zip_zero_struct(&gArchive);
    return false;
  }

  Log::info("Mounted Sinen archive {}", gArchivePath.c_str());
  return true;
}

void AssetIO::unmountArchive() {
  std::lock_guard<std::mutex> lock(gArchiveMutex);
  if (!gArchiveMounted) {
    return;
  }
  mz_zip_reader_end(&gArchive);
  mz_zip_zero_struct(&gArchive);
  gArchiveMounted = false;
  gArchivePath.clear();
}

bool AssetIO::isArchiveMounted() {
  std::lock_guard<std::mutex> lock(gArchiveMutex);
  return gArchiveMounted;
}

bool AssetIO::isArchivePath(StringView path) {
  const String normalized = normalizePath(path);
  return normalized == archiveRootDirectory() ||
         normalized.starts_with(String(archiveRootDirectory()) + "/");
}

bool AssetIO::exists(StringView name) {
  if (!name.starts_with("user://") && isArchiveMounted()) {
    const String path = archiveEntryPath(name);
    if (!path.empty() && archiveEntryExists(path)) {
      return true;
    }
  }

  String filePath;
  if (!convertFilePath(filePath, name, FilesystemAccess::Read)) {
    return false;
  }
  auto *file = SDL_IOFromFile(filePath.c_str(), "rb");
  if (!file) {
    return false;
  }
  SDL_CloseIO(file);
  return true;
}

bool AssetIO::archiveEntryExists(StringView path) {
  std::lock_guard<std::mutex> lock(gArchiveMutex);
  return archiveEntryExistsLocked(path);
}

bool AssetIO::archiveDirectoryExists(StringView path) {
  std::lock_guard<std::mutex> lock(gArchiveMutex);
  return archiveDirectoryExistsLocked(path);
}

String AssetIO::openArchiveEntryAsString(StringView path) {
  std::lock_guard<std::mutex> lock(gArchiveMutex);
  return openArchiveEntryAsStringLocked(path);
}

Array<String> AssetIO::enumerateArchiveDirectory(StringView path) {
  Array<String> result;
  std::lock_guard<std::mutex> lock(gArchiveMutex);
  if (!gArchiveMounted) {
    return result;
  }

  String normalized = archiveEntryPath(path);
  if (normalized.empty()) {
    return result;
  }
  if (!normalized.empty() && normalized.back() != '/') {
    normalized.push_back('/');
  }

  const mz_uint count = mz_zip_reader_get_num_files(&gArchive);
  for (mz_uint i = 0; i < count; ++i) {
    mz_zip_archive_file_stat stat{};
    if (!mz_zip_reader_file_stat(&gArchive, i, &stat)) {
      continue;
    }
    const String name = normalizePath(stat.m_filename);
    if (!name.starts_with(normalized) || name.size() <= normalized.size()) {
      continue;
    }
    String rest = name.substr(normalized.size());
    const size_t slash = rest.find('/');
    if (slash != String::npos) {
      rest = rest.substr(0, slash);
    }
    if (!rest.empty() &&
        std::find(result.begin(), result.end(), rest) == result.end()) {
      result.push_back(rest);
    }
  }
  return result;
}

} // namespace sinen
