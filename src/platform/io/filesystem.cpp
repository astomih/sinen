#include "core/logger/log.hpp"

#include <platform/io/asset_reader.hpp>
#include <platform/io/filesystem.hpp>
#include <script/script.hpp>

#include <SDL3/SDL.h>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <string_view>
#include <system_error>
#include <utility>

namespace sinen {
namespace {
std::filesystem::path
normalizeFsPath(const std::filesystem::path &path) {
  std::error_code ec;
  std::filesystem::path normalized =
      std::filesystem::weakly_canonical(path, ec);
  if (ec) {
    normalized = std::filesystem::absolute(path, ec);
    if (ec) {
      normalized = path;
    }
  }
  return normalized.lexically_normal();
}

bool samePathComponent(StringView a, StringView b) {
#ifdef _WIN32
  if (a.size() != b.size()) {
    return false;
  }
  for (size_t i = 0; i < a.size(); ++i) {
    const auto ca = static_cast<unsigned char>(a[i]);
    const auto cb = static_cast<unsigned char>(b[i]);
    if (std::tolower(ca) != std::tolower(cb)) {
      return false;
    }
  }
  return true;
#else
  return a == b;
#endif
}

bool isPathUnderRoot(const std::filesystem::path &path,
                            const std::filesystem::path &root) {
  auto p = path.lexically_normal();
  auto r = root.lexically_normal();

  auto pit = p.begin();
  auto rit = r.begin();
  for (; rit != r.end(); ++rit, ++pit) {
    if (pit == p.end()) {
      return false;
    }

    const auto a = pit->generic_string();
    const auto b = rit->generic_string();
    if (!samePathComponent(StringView(a.data(), a.size()),
                           StringView(b.data(), b.size()))) {
      return false;
    }
  }
  return true;
}

bool resolvePathUnderRoot(const std::filesystem::path &requested,
                                 const std::filesystem::path &root,
                                 String &resolvedPath) {
  std::filesystem::path candidate =
      requested.is_absolute() ? requested : (root / requested);
  candidate = normalizeFsPath(candidate);

  if (!isPathUnderRoot(candidate, root)) {
    return false;
  }

  const auto s = candidate.string();
  resolvedPath = String(s.c_str(), s.size());
  return true;
}

bool startsWithUserScheme(StringView path) {
  constexpr StringView scheme = "user://";
  return path.size() >= scheme.size() &&
         path.substr(0, scheme.size()) == scheme;
}

bool startsWithLogicalRoot(StringView path) {
  return !path.empty() && (path[0] == '/' || path[0] == '\\');
}

String stripLogicalRoot(StringView path) {
  while (!path.empty() && (path[0] == '/' || path[0] == '\\')) {
    path.remove_prefix(1);
  }
  return String(path);
}

std::filesystem::path appBaseRootPath() {
  return normalizeFsPath(
      std::filesystem::path(Filesystem::getAppBaseDirectory().c_str()));
}

bool resolveAppSandboxPath(const std::filesystem::path &requested,
                                  String &resolvedPath) {
#ifdef SINEN_DEBUG_WORKING_DIRECTORY
  std::error_code cwdEc;
  const std::filesystem::path workingRoot =
      normalizeFsPath(std::filesystem::current_path(cwdEc));
  if (!cwdEc && !workingRoot.empty() &&
      resolvePathUnderRoot(requested, workingRoot, resolvedPath)) {
    return true;
  }
#endif

  const std::filesystem::path baseRoot = appBaseRootPath();
  return !baseRoot.empty() &&
         resolvePathUnderRoot(requested, baseRoot, resolvedPath);
}

bool resolveAppLogicalDirectory(StringView path, String &resolvedPath) {
  const std::filesystem::path requested{String(path)};
  return resolveAppSandboxPath(requested, resolvedPath);
}

bool resolveUserLogicalPath(StringView path, String &resolvedPath) {
  constexpr StringView scheme = "user://";
  String rest = String(path.substr(scheme.size()));
  rest = stripLogicalRoot(rest);

  const String userDir = Filesystem::getUserDirectory();
  if (userDir.empty()) {
    resolvedPath.clear();
    return false;
  }

  const std::filesystem::path userRoot =
      normalizeFsPath(std::filesystem::path(userDir.c_str()));
  if (userRoot.empty()) {
    resolvedPath.clear();
    return false;
  }
  return resolvePathUnderRoot(std::filesystem::path(rest.c_str()), userRoot,
                              resolvedPath);
}

bool resolveAppLogicalPath(StringView path, String &resolvedPath) {
  const String basePath = Script::getBaseDirectory();
  if (!startsWithLogicalRoot(path) && startsWithUserScheme(basePath)) {
    String userPath = basePath;
    if (!userPath.empty() && userPath.back() != '/' &&
        userPath.back() != '\\') {
      userPath.push_back('/');
    }
    userPath += String(path);
    return resolveUserLogicalPath(userPath, resolvedPath);
  }

  String rootResolved;
  if (!resolveAppLogicalDirectory(".", rootResolved)) {
    resolvedPath.clear();
    return false;
  }

  String logicalPath = startsWithLogicalRoot(path)
                           ? stripLogicalRoot(path)
                           : (basePath + "/" + String(path));
  logicalPath = stripLogicalRoot(logicalPath);
  return resolvePathUnderRoot(std::filesystem::path(logicalPath.c_str()),
                              std::filesystem::path(rootResolved.c_str()),
                              resolvedPath);
}

#if defined(SINEN_PLATFORM_EMSCRIPTEN) || defined(EMSCRIPTEN)
static String normalizeVirtualPath(StringView path) {
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

  String out = "/";
  for (size_t i = 0; i < parts.size(); ++i) {
    if (i > 0) {
      out.push_back('/');
    }
    out += parts[i];
  }
  return out;
}
#endif
} // namespace

Array<String> Filesystem::enumerateDirectory(StringView path) {
  if (!startsWithUserScheme(path) && AssetReader::isArchiveMounted()) {
    return AssetReader::enumerateArchiveDirectory(path);
  }

  String resolvedPath;
  if (!resolveSandboxPath(path, FilesystemAccess::Read, resolvedPath)) {
    Log::error("Filesystem sandbox rejected directory: {}",
               String(path).c_str());
    return {};
  }

  Array<String> result;
  SDL_EnumerateDirectory(
      resolvedPath.c_str(),
      [](void *userdata, const char *dirname, const char *fname) {
        (void)dirname;
        auto *vec = reinterpret_cast<Array<String> *>(userdata);
        if (fname == nullptr)
          return SDL_ENUM_SUCCESS;
        vec->emplace_back(fname);
        return SDL_ENUM_CONTINUE;
      },
      &result);
  return result;
}

std::optional<Buffer> Filesystem::read(StringView path) {
  if (!startsWithUserScheme(path) && AssetReader::isArchiveMounted() &&
      AssetReader::exists(path)) {
    const String data = AssetReader::readAsString(path);
    Buffer buffer = makeBuffer(data.size(), BufferType::Binary);
    if (!data.empty()) {
      std::memcpy(buffer.data(), data.data(), data.size());
    }
    return buffer;
  }

  String resolvedPath;
  if (!resolveSandboxPath(path, FilesystemAccess::Read, resolvedPath)) {
    Log::error("Filesystem sandbox rejected read: {}", String(path).c_str());
    return std::nullopt;
  }

  auto *file = SDL_IOFromFile(resolvedPath.c_str(), "rb");
  if (!file) {
    Log::error("Filesystem open error {}: {}", resolvedPath.c_str(),
               SDL_GetError());
    return std::nullopt;
  }

  size_t fileLength = 0;
  void *loaded = SDL_LoadFile_IO(file, &fileLength, 1);
  if (!loaded) {
    Log::error("Filesystem read error {}: {}", resolvedPath.c_str(),
               SDL_GetError());
    return std::nullopt;
  }

  Buffer buffer = makeBuffer(fileLength, BufferType::Binary);
  if (fileLength > 0) {
    std::memcpy(buffer.data(), loaded, fileLength);
  }
  SDL_free(loaded);
  return buffer;
}

bool Filesystem::exists(StringView path) {
  if (!startsWithUserScheme(path) && AssetReader::isArchiveMounted() &&
      AssetReader::exists(path)) {
    return true;
  }

  String resolvedPath;
  if (!resolveSandboxPath(path, FilesystemAccess::Read, resolvedPath)) {
    return false;
  }

  auto *file = SDL_IOFromFile(resolvedPath.c_str(), "rb");
  if (!file) {
    return false;
  }
  SDL_CloseIO(file);
  return true;
}

bool Filesystem::write(StringView path, const Buffer &buffer) {
  String resolvedPath;
  if (!resolveSandboxPath(path, FilesystemAccess::Write, resolvedPath)) {
    Log::error("Filesystem sandbox rejected write: {}", String(path).c_str());
    return false;
  }

  auto *file = SDL_IOFromFile(resolvedPath.c_str(), "wb");
  if (!file) {
    Log::error("Filesystem open error {}: {}", resolvedPath.c_str(),
               SDL_GetError());
    return false;
  }

  const size_t written = SDL_WriteIO(file, buffer.data(), buffer.size());
  SDL_CloseIO(file);
  if (written != static_cast<size_t>(buffer.size())) {
    Log::error("Filesystem write error {}: {}", resolvedPath.c_str(),
               SDL_GetError());
    return false;
  }
  return true;
}

String Filesystem::getAppBaseDirectory() {

#ifdef __ANDROID__
  String base = "/sdcard/Android/media/astomih.sinen.app";
#else
  const char *basePath = SDL_GetBasePath();
  String base = basePath ? basePath : ".";
#endif
  return base;
}

String Filesystem::getUserDirectory() {
#if defined(SINEN_PLATFORM_EMSCRIPTEN) || defined(EMSCRIPTEN)
  return "";
#elif defined(__ANDROID__)
  return "/sdcard/Android/media/astomih.sinen.app";
#else
  char *prefPath = SDL_GetPrefPath("astomih", "sinen");
  if (!prefPath) {
    return "";
  }
  String result(prefPath);
  SDL_free(prefPath);
  return result;
#endif
}

bool Filesystem::resolveSandboxPath(StringView path, FilesystemAccess access,
                                    String &resolvedPath) {
  (void)access;

#if defined(SINEN_PLATFORM_EMSCRIPTEN) || defined(EMSCRIPTEN)
  if (startsWithUserScheme(path)) {
    resolvedPath.clear();
    return false;
  }
  const String basePath = startsWithLogicalRoot(path)
                              ? "/"
                              : Script::getBaseDirectory();
  if (!startsWithLogicalRoot(path) && startsWithUserScheme(basePath)) {
    resolvedPath.clear();
    return false;
  }
  const String logicalPath =
      startsWithLogicalRoot(path) ? stripLogicalRoot(path) : String(path);
  resolvedPath =
      normalizeVirtualPath(stripLogicalRoot(basePath + "/" + logicalPath));
  return true;
#else
  if (startsWithUserScheme(path)) {
    if (resolveUserLogicalPath(path, resolvedPath)) {
      return true;
    }
    resolvedPath.clear();
    return false;
  }

  if (resolveAppLogicalPath(path, resolvedPath)) {
    return true;
  }

  resolvedPath.clear();
  return false;
#endif
}

bool Filesystem::isSandboxPathAllowed(StringView path,
                                      FilesystemAccess access) {
  String resolvedPath;
  return resolveSandboxPath(path, access, resolvedPath);
}

} // namespace sinen
