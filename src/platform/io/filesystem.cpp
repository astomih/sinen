#include "core/logger/log.hpp"

#include <platform/io/asset_io.hpp>
#include <platform/io/filesystem.hpp>
#include <script/luaapi.hpp>

#include <SDL3/SDL.h>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <system_error>
#include <utility>

namespace sinen {
namespace {
static std::filesystem::path
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

static bool samePathComponent(StringView a, StringView b) {
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

static bool isPathUnderRoot(const std::filesystem::path &path,
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

static bool resolvePathUnderRoot(const std::filesystem::path &requested,
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
  if (AssetIO::isArchiveMounted()) {
    return AssetIO::enumerateArchiveDirectory(path);
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
  if (AssetIO::isArchiveMounted() && AssetIO::exists(path)) {
    const String data = AssetIO::openAsString(path);
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
  if (AssetIO::isArchiveMounted() && AssetIO::exists(path)) {
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
#ifdef _WIN32
  char *prefPath = SDL_GetPrefPath("astomih", "sinen");
  if (!prefPath) {
    return "";
  }
  String result(prefPath);
  SDL_free(prefPath);
  return result;
#else
  return "";
#endif
}

bool Filesystem::resolveSandboxPath(StringView path, FilesystemAccess access,
                                    String &resolvedPath) {
  (void)access;

#if defined(SINEN_PLATFORM_EMSCRIPTEN) || defined(EMSCRIPTEN)
  resolvedPath = normalizeVirtualPath(path);
  return true;
#else
  const std::filesystem::path baseRoot =
      normalizeFsPath(std::filesystem::path(getAppBaseDirectory().c_str()));
  if (baseRoot.empty()) {
    return false;
  }

  std::filesystem::path requested{String(path)};

#ifdef SINEN_DEBUG_WORKING_DIRECTORY
  std::error_code cwdEc;
  const std::filesystem::path workingRoot =
      normalizeFsPath(std::filesystem::current_path(cwdEc));
  if (!cwdEc && !workingRoot.empty() &&
      resolvePathUnderRoot(requested, workingRoot, resolvedPath)) {
    return true;
  }
#endif

  if (resolvePathUnderRoot(requested, baseRoot, resolvedPath)) {
    return true;
  }

#ifdef _WIN32
  const String userDir = getUserDirectory();
  if (!userDir.empty()) {
    const std::filesystem::path userRoot =
        normalizeFsPath(std::filesystem::path(userDir.c_str()));
    if (!userRoot.empty() &&
        resolvePathUnderRoot(requested, userRoot, resolvedPath)) {
      return true;
    }
  }
#endif

  resolvedPath.clear();
  return false;
#endif
}

bool Filesystem::isSandboxPathAllowed(StringView path,
                                      FilesystemAccess access) {
  String resolvedPath;
  return resolveSandboxPath(path, access, resolvedPath);
}

static int lFilesystemEnumerateDirectory(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  auto list = Filesystem::enumerateDirectory(StringView(path));
  lua_newtable(L);
  for (size_t i = 0; i < list.size(); ++i) {
    lua_pushlstring(L, list[i].data(), list[i].size());
    lua_rawseti(L, -2, static_cast<lua_Integer>(i + 1));
  }
  return 1;
}
static int lFilesystemRead(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  auto buffer = Filesystem::read(StringView(path));
  if (!buffer) {
    lua_pushnil(L);
    return 1;
  }
  udNewOwned<Buffer>(L, std::move(*buffer));
  return 1;
}
static int lFilesystemExists(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  lua_pushboolean(L, Filesystem::exists(StringView(path)));
  return 1;
}
static int lFilesystemWrite(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const auto &buffer = udValue<Buffer>(L, 2);
  lua_pushboolean(L, Filesystem::write(StringView(path), buffer));
  return 1;
}
static int lFilesystemGetUserDirectory(lua_State *L) {
  auto path = Filesystem::getUserDirectory();
  lua_pushlstring(L, path.data(), path.size());
  return 1;
}
void registerFilesystem(lua_State *L) {
  pushSnNamed(L, "Filesystem");
  luaPushcfunction2(L, lFilesystemEnumerateDirectory);
  lua_setfield(L, -2, "enumerateDirectory");
  luaPushcfunction2(L, lFilesystemRead);
  lua_setfield(L, -2, "read");
  luaPushcfunction2(L, lFilesystemExists);
  lua_setfield(L, -2, "exists");
  luaPushcfunction2(L, lFilesystemWrite);
  lua_setfield(L, -2, "write");
  luaPushcfunction2(L, lFilesystemGetUserDirectory);
  lua_setfield(L, -2, "getUserDirectory");
  lua_pop(L, 1);
}
} // namespace sinen
