#include "native_module.hpp"

#include <script/sinen_lua_module.h>

#include <lua.h>

#include <cstring>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

namespace sinen {
namespace {

#if defined(_WIN32)
using LibraryHandle = HMODULE;
#else
using LibraryHandle = void *;
#endif

struct LoadedModule {
  std::string path;
  LibraryHandle handle = nullptr;
  sinen_lua_module_close_fn close = nullptr;
  bool initialized = false;
  std::string loadError;
};

template <class Function> Function symbolAsFunction(void *symbol) {
  static_assert(sizeof(Function) == sizeof(symbol));
  Function function = nullptr;
  std::memcpy(&function, &symbol, sizeof(function));
  return function;
}

#if defined(_WIN32)
std::string windowsErrorMessage(DWORD code) {
  wchar_t *message = nullptr;
  const DWORD size = FormatMessageW(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
          FORMAT_MESSAGE_IGNORE_INSERTS,
      nullptr, code, 0, reinterpret_cast<wchar_t *>(&message), 0, nullptr);
  if (size == 0 || !message) {
    return "Windows error " + std::to_string(code);
  }

  const int utf8Size =
      WideCharToMultiByte(CP_UTF8, 0, message, static_cast<int>(size), nullptr,
                          0, nullptr, nullptr);
  std::string result(static_cast<size_t>(utf8Size), '\0');
  if (utf8Size > 0) {
    WideCharToMultiByte(CP_UTF8, 0, message, static_cast<int>(size),
                        result.data(), utf8Size, nullptr, nullptr);
  }
  LocalFree(message);

  while (!result.empty() && (result.back() == '\r' || result.back() == '\n' ||
                             result.back() == ' ')) {
    result.pop_back();
  }
  return result;
}

LibraryHandle openLibrary(std::string_view path, std::string &error) {
  std::error_code filesystemError;
  std::filesystem::path libraryPath =
      std::filesystem::absolute(std::filesystem::u8path(path), filesystemError);
  if (filesystemError) {
    error = "cannot resolve module path: " + filesystemError.message();
    return nullptr;
  }

  LibraryHandle handle = LoadLibraryExW(libraryPath.c_str(), nullptr,
                                        LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR |
                                            LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
  if (!handle) {
    error = windowsErrorMessage(GetLastError());
  }
  return handle;
}

void *findSymbol(LibraryHandle handle, const char *name) {
  FARPROC symbol = GetProcAddress(handle, name);
  void *result = nullptr;
  static_assert(sizeof(symbol) == sizeof(result));
  std::memcpy(&result, &symbol, sizeof(result));
  return result;
}

void closeLibrary(LibraryHandle handle) { FreeLibrary(handle); }
#else
LibraryHandle openLibrary(std::string_view path, std::string &error) {
  std::string nullTerminatedPath(path);
  dlerror();
  LibraryHandle handle =
      dlopen(nullTerminatedPath.c_str(), RTLD_NOW | RTLD_LOCAL);
  if (!handle) {
    const char *message = dlerror();
    error = message ? message : "dlopen failed";
  }
  return handle;
}

void *findSymbol(LibraryHandle handle, const char *name) {
  dlerror();
  return dlsym(handle, name);
}

void closeLibrary(LibraryHandle handle) { dlclose(handle); }
#endif

const char *nativeModuleExtension() {
#if defined(_WIN32)
  return ".dll";
#elif defined(__EMSCRIPTEN__)
  return ".wasm";
#elif defined(__APPLE__)
  return ".dylib";
#else
  return ".so";
#endif
}

std::string pathToUtf8(const std::filesystem::path &path) {
  const auto encoded = path.u8string();
  return std::string(reinterpret_cast<const char *>(encoded.data()),
                     encoded.size());
}

int callModuleFunction(lua_State *state, sinen_lua_module_open_fn function) {
  return function(state);
}

int moduleFunctionThunk(lua_State *state) {
  auto *storedFunction = static_cast<sinen_lua_module_open_fn *>(
      lua_touserdata(state, lua_upvalueindex(1)));
  return callModuleFunction(state, *storedFunction);
}

bool protectedModuleCall(lua_State *state, sinen_lua_module_open_fn function,
                         std::string &error) {
  const int stackTop = lua_gettop(state);
  auto *storedFunction = static_cast<sinen_lua_module_open_fn *>(
      lua_newuserdata(state, sizeof(function)));
  std::memcpy(storedFunction, &function, sizeof(function));
  lua_pushcclosure(state, moduleFunctionThunk, "sinen native module", 1);

  const int status = lua_pcall(state, 0, 0, 0);
  if (status != LUA_OK) {
    const char *message = lua_tostring(state, -1);
    error = message ? message : "native module raised an unknown Lua error";
    lua_settop(state, stackTop);
    return false;
  }

  lua_settop(state, stackTop);
  return true;
}

} // namespace

struct NativeModuleManager::Impl {
  std::vector<LoadedModule> modules;
  std::string error;
};

NativeModuleManager::Impl &NativeModuleManager::impl() {
  static Impl value;
  return value;
}

bool NativeModuleManager::load(lua_State *state, std::string_view path) {
  Impl &data = impl();
  data.error.clear();
  if (!state) {
    data.error = "Lua state is null";
    return false;
  }
  if (path.empty()) {
    data.error = "native module path is empty";
    return false;
  }

  for (const LoadedModule &module : data.modules) {
    if (module.path == path) {
      if (module.initialized) {
        return true;
      }
      data.error = module.loadError;
      return false;
    }
  }

  LibraryHandle handle = openLibrary(path, data.error);
  if (!handle) {
    return false;
  }

  auto fail = [&](std::string message) {
    data.error = std::move(message);
    closeLibrary(handle);
    return false;
  };

  auto getVersion = symbolAsFunction<sinen_lua_module_get_api_version_fn>(
      findSymbol(handle, SINEN_LUA_MODULE_VERSION_SYMBOL));
  if (!getVersion) {
    return fail("missing required export '" +
                std::string(SINEN_LUA_MODULE_VERSION_SYMBOL) + "'");
  }

  const uint32_t version = getVersion();
  if (version != SINEN_LUA_MODULE_API_VERSION) {
    return fail("unsupported native module API version " +
                std::to_string(version) + " (expected " +
                std::to_string(SINEN_LUA_MODULE_API_VERSION) + ")");
  }

  auto open = symbolAsFunction<sinen_lua_module_open_fn>(
      findSymbol(handle, SINEN_LUA_MODULE_OPEN_SYMBOL));
  if (!open) {
    return fail("missing required export '" +
                std::string(SINEN_LUA_MODULE_OPEN_SYMBOL) + "'");
  }

  auto close = symbolAsFunction<sinen_lua_module_close_fn>(
      findSymbol(handle, SINEN_LUA_MODULE_CLOSE_SYMBOL));
  if (!protectedModuleCall(state, open, data.error)) {
    // open may have registered C closures before it failed. Keep the library
    // alive until the Lua state closes so those pointers never dangle.
    data.modules.push_back(
        LoadedModule{std::string(path), handle, close, false, data.error});
    return false;
  }

  data.modules.push_back(
      LoadedModule{std::string(path), handle, close, true, {}});
  return true;
}

bool NativeModuleManager::loadPlugin(lua_State *state, std::string_view name) {
  Impl &data = impl();
  data.error.clear();

  if (name.empty()) {
    data.error = "plugin name is empty";
    return false;
  }
  if (name.find('\0') != std::string_view::npos) {
    data.error = "plugin name contains a null byte";
    return false;
  }

  // Plugin lookup is deliberately a name-only operation. In particular,
  // absolute paths and directory traversal must not turn this API into a
  // general-purpose native library loader.
  if (name.find('/') != std::string_view::npos ||
      name.find('\\') != std::string_view::npos) {
    data.error = "plugin name must not contain a directory separator";
    return false;
  }

  std::filesystem::path fileName;
  try {
    fileName = std::filesystem::u8path(name);
  } catch (const std::filesystem::filesystem_error &error) {
    data.error = "invalid plugin name: " + std::string(error.what());
    return false;
  }
  if (!fileName.has_filename() || fileName.has_root_path() ||
      fileName.filename() != fileName) {
    data.error = "plugin name must name a file in the current directory";
    return false;
  }
  if (!fileName.has_extension()) {
    fileName += nativeModuleExtension();
  }

  std::error_code filesystemError;
  const std::filesystem::path currentDirectory =
      std::filesystem::current_path(filesystemError);
  if (filesystemError) {
    data.error =
        "cannot resolve current directory: " + filesystemError.message();
    return false;
  }

  try {
    return load(state, pathToUtf8(currentDirectory / fileName));
  } catch (const std::filesystem::filesystem_error &error) {
    data.error = "cannot resolve plugin path: " + std::string(error.what());
    return false;
  }
}

void NativeModuleManager::close(lua_State *state) {
  Impl &data = impl();
  for (auto module = data.modules.rbegin(); module != data.modules.rend();
       ++module) {
    if (!module->initialized || !module->close) {
      continue;
    }
    std::string closeError;
    if (!protectedModuleCall(state, module->close, closeError)) {
      data.error = "native module close failed for '" + module->path +
                   "': " + closeError;
    }
  }
}

void NativeModuleManager::unload() {
  Impl &data = impl();
  for (auto module = data.modules.rbegin(); module != data.modules.rend();
       ++module) {
    closeLibrary(module->handle);
  }
  data.modules.clear();
}

const std::string &NativeModuleManager::lastError() const {
  return impl().error;
}

NativeModuleManager &nativeModuleManager() {
  static NativeModuleManager manager;
  return manager;
}

} // namespace sinen
