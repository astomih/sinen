#include "external_file_lua.hpp"
#include "luaapi.hpp"

#include <platform/io/external_file.hpp>
#include <platform/window/window_native.hpp>

#include <SDL3/SDL_dialog.h>

#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

namespace sinen {
namespace {
struct DialogRequest {
  int callbackRef = LUA_NOREF;
  bool cancelled = false;
  std::vector<std::string> filterNames;
  std::vector<std::string> filterPatterns;
  std::vector<SDL_DialogFileFilter> filters;
};

struct DialogCompletion {
  DialogRequest *request = nullptr;
  std::vector<std::string> paths;
  std::string error;
};

std::mutex gExternalFileMutex;
std::unordered_set<DialogRequest *> gDialogRequests;
std::vector<DialogCompletion> gDialogCompletions;
std::vector<std::string> gDroppedPaths;

void pushExternalFile(lua_State *L, String path) {
  udNewOwned<ExternalFile>(L, ExternalFile(std::move(path)));
}

void SDLCALL onOpenFileDialog(void *userdata,
                              const char *const *filelist, int filter) {
  (void)filter;
  auto *request = static_cast<DialogRequest *>(userdata);
  DialogCompletion completion;
  completion.request = request;

  if (!filelist) {
    const char *message = SDL_GetError();
    completion.error = message ? message : "File dialog failed";
  } else {
    for (size_t i = 0; filelist[i]; ++i) {
      completion.paths.emplace_back(filelist[i]);
    }
  }

  bool deleteRequest = false;
  {
    std::lock_guard<std::mutex> lock(gExternalFileMutex);
    if (request->cancelled) {
      gDialogRequests.erase(request);
      deleteRequest = true;
    } else {
      gDialogCompletions.push_back(std::move(completion));
    }
  }
  if (deleteRequest) {
    delete request;
  }
}

static int lExternalFileRead(lua_State *L) {
  const auto &file = udValue<ExternalFile>(L, 1);
  auto buffer = file.read();
  if (!buffer) {
    lua_pushnil(L);
    return 1;
  }
  udNewOwned<Buffer>(L, std::move(*buffer));
  return 1;
}

static int lExternalFileName(lua_State *L) {
  const auto &file = udValue<ExternalFile>(L, 1);
  const String name = file.name();
  lua_pushlstring(L, name.data(), name.size());
  return 1;
}

static int lExternalFileExtension(lua_State *L) {
  const auto &file = udValue<ExternalFile>(L, 1);
  const String extension = file.extension();
  lua_pushlstring(L, extension.data(), extension.size());
  return 1;
}

static int lExternalFileTostring(lua_State *L) {
  const auto &file = udValue<ExternalFile>(L, 1);
  const String text = "ExternalFile(" + file.name() + ")";
  lua_pushlstring(L, text.data(), text.size());
  return 1;
}

static int lFileDialogOpen(lua_State *L) {
  luaL_checktype(L, 1, LUA_TFUNCTION);

  std::vector<std::string> filterNames;
  std::vector<std::string> filterPatterns;
  if (!lua_isnoneornil(L, 2)) {
    luaL_checktype(L, 2, LUA_TTABLE);
    const size_t count = lua_objlen(L, 2);
    filterNames.reserve(count);
    filterPatterns.reserve(count);
    for (size_t i = 1; i <= count; ++i) {
      lua_rawgeti(L, 2, static_cast<lua_Integer>(i));
      luaL_checktype(L, -1, LUA_TTABLE);
      lua_getfield(L, -1, "name");
      filterNames.emplace_back(luaL_checkstring(L, -1));
      lua_pop(L, 1);
      lua_getfield(L, -1, "pattern");
      filterPatterns.emplace_back(luaL_checkstring(L, -1));
      lua_pop(L, 2);
    }
  }

  auto *request = new DialogRequest();
  request->filterNames = std::move(filterNames);
  request->filterPatterns = std::move(filterPatterns);
  request->filters.reserve(request->filterNames.size());
  for (size_t i = 0; i < request->filterNames.size(); ++i) {
    request->filters.push_back(SDL_DialogFileFilter{
        request->filterNames[i].c_str(), request->filterPatterns[i].c_str()});
  }

  lua_pushvalue(L, 1);
  request->callbackRef = luaLRef2(L, -1);
  {
    std::lock_guard<std::mutex> lock(gExternalFileMutex);
    gDialogRequests.insert(request);
  }

  SDL_ShowOpenFileDialog(
      onOpenFileDialog, request, WindowNative::getWindow(),
      request->filters.empty() ? nullptr : request->filters.data(),
      static_cast<int>(request->filters.size()), nullptr,
      lua_toboolean(L, 3) != 0);
  return 0;
}
} // namespace

void registerExternalFile(lua_State *L) {
  luaL_newmetatable(L, ExternalFile::metaTableName());
  Binding::registerFunction(L, "__gc", udGc<ExternalFile>);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  Binding::registerFunction(L, "read", lExternalFileRead);
  Binding::registerFunction(L, "name", lExternalFileName);
  Binding::registerFunction(L, "extension", lExternalFileExtension);
  Binding::registerFunction(L, "__tostring", lExternalFileTostring);
  lua_pop(L, 1);

  // Expose the type namespace without a constructor. ExternalFile values can
  // only originate from a dialog selection or a drop event.
  pushSnNamed(L, "ExternalFile");
  lua_pop(L, 1);

  pushSnNamed(L, "FileDialog");
  Binding::registerFunction(L, "open", lFileDialogOpen);
  lua_pop(L, 1);
}

void resetExternalFileCallbacks(lua_State *L) {
  std::vector<int> callbackRefs;
  std::vector<DialogRequest *> completedRequests;
  {
    std::lock_guard<std::mutex> lock(gExternalFileMutex);
    gDroppedPaths.clear();
    for (auto *request : gDialogRequests) {
      request->cancelled = true;
      if (request->callbackRef != LUA_NOREF) {
        callbackRefs.push_back(request->callbackRef);
        request->callbackRef = LUA_NOREF;
      }
    }
    for (const auto &completion : gDialogCompletions) {
      gDialogRequests.erase(completion.request);
      completedRequests.push_back(completion.request);
    }
    gDialogCompletions.clear();
  }
  for (const int callbackRef : callbackRefs) {
    luaLUnref2(L, LUA_REGISTRYINDEX, callbackRef);
  }
  for (auto *request : completedRequests) {
    delete request;
  }
}

void pumpExternalFileCallbacks(lua_State *L, int fileDroppedRef) {
  std::vector<DialogCompletion> completions;
  std::vector<std::string> droppedPaths;
  {
    std::lock_guard<std::mutex> lock(gExternalFileMutex);
    completions.swap(gDialogCompletions);
    droppedPaths.swap(gDroppedPaths);
    for (const auto &completion : completions) {
      gDialogRequests.erase(completion.request);
    }
  }

  for (auto &completion : completions) {
    DialogRequest *request = completion.request;
    if (!request->cancelled && request->callbackRef != LUA_NOREF) {
      lua_rawgeti(L, LUA_REGISTRYINDEX, request->callbackRef);
      lua_createtable(L, static_cast<int>(completion.paths.size()), 0);
      for (size_t i = 0; i < completion.paths.size(); ++i) {
        pushExternalFile(L, String(completion.paths[i]));
        lua_rawseti(L, -2, static_cast<lua_Integer>(i + 1));
      }
      if (completion.error.empty()) {
        lua_pushnil(L);
      } else {
        lua_pushlstring(L, completion.error.data(), completion.error.size());
      }
      luaPCallLogged(L, 2, 0);
      luaLUnref2(L, LUA_REGISTRYINDEX, request->callbackRef);
    }
    delete request;
  }

  if (fileDroppedRef == LUA_NOREF) {
    return;
  }
  for (auto &path : droppedPaths) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, fileDroppedRef);
    pushExternalFile(L, String(path));
    luaPCallLogged(L, 1, 0);
  }
}

void enqueueDroppedExternalFile(StringView path) {
  if (path.empty()) {
    return;
  }
  std::lock_guard<std::mutex> lock(gExternalFileMutex);
  gDroppedPaths.emplace_back(path);
}

} // namespace sinen
