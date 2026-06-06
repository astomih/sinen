// internal
#include "script.hpp"

#include <core/allocator/global_allocator.hpp>
#include <core/buffer/buffer.hpp>
#include <core/core.hpp>
#include <core/data/ptr.hpp>
#include <core/data/string.hpp>
#include <core/logger/log.hpp>
#include <core/thread/load_context.hpp>
#include <core/thread/task_group.hpp>
#include <graphics/graphics.hpp>
#include <platform/io/asset_reader.hpp>
#include <platform/io/filesystem.hpp>
#include <platform/window/window.hpp>

#include "luaapi.hpp"
#include "require.hpp"
#include <Luau/Require.h>

#include <debugger.h>

#include <SDL3/SDL.h>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <optional>
#include <string_view>

namespace sinen {
auto alloc = [](void *ud, void *ptr, size_t osize, size_t nsize) -> void * {
  (void)ud;
  // free
  if (nsize == 0) {
    if (ptr)
      sinen::GlobalAllocator::get()->deallocate(ptr, osize);
    return nullptr;
  }

  if (ptr == nullptr) {
    return sinen::GlobalAllocator::get()->allocate(nsize);
  }

  void *nptr = sinen::GlobalAllocator::get()->allocate(nsize);
  if (!nptr) {
    return nullptr;
  }

  std::memcpy(nptr, ptr, (osize < nsize) ? osize : nsize);
  sinen::GlobalAllocator::get()->deallocate(ptr, osize);
  return nptr;
};
luau::debugger::Debugger debugger(false);
void luaPushcfunction2(lua_State *L, lua_CFunction f) {
  lua_pushcfunction(L, f, "sn function");
}
int luaLError2(lua_State *L, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  va_list ap2;
  va_copy(ap2, ap);
  int n = std::vsnprintf(nullptr, 0, fmt, ap2);
  va_end(ap2);

  if (n < 0) {
    luaL_errorL(L, "luaL_verrorL: formatting failed");
    return -1;
  }

  std::vector<char> buf(static_cast<size_t>(n) + 1);
  std::vsnprintf(buf.data(), buf.size(), fmt, ap);

  luaL_errorL(L, "%s", buf.data());

  return 0;
}
int luaLRef2(lua_State *L, int idx) {
  int ref = lua_ref(L, idx);
  lua_remove(L, idx);
  return ref;
}
void luaLUnref2(lua_State *L, int idx, int r) { lua_unref(L, r); }
void pushSnNamed(lua_State *L, const char *name) {
  lua_getglobal(L, "sn");
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "sn");
  }
  lua_newtable(L);
  lua_pushvalue(L, -1);
  lua_setfield(L, -3, name);
  lua_remove(L, -2);
}
static lua_State *gLua = nullptr;
lua_State *getGlobalLua() { return gLua; }
static int gSetupRef = LUA_NOREF;
static int gUpdateRef = LUA_NOREF;
static int gDrawRef = LUA_NOREF;

enum class ScriptScenePhase {
  Running,
  Loading,
};
static ScriptScenePhase gScenePhase = ScriptScenePhase::Running;
static TaskGroup gSetupTasks;

static int requireWithSinenAlias(lua_State *L) {
  const char *moduleName = luaL_checkstring(L, 1);
  if (std::strcmp(moduleName, "@sinen") == 0) {
    lua_getglobal(L, "sn");
    return 1;
  }

  lua_pushvalue(L, lua_upvalueindex(1));
  lua_insert(L, 1);

  const int nargs = lua_gettop(L) - 1;
  lua_call(L, nargs, LUA_MULTRET);
  return lua_gettop(L);
}

static void installRequireAlias(lua_State *L) {
  lua_getglobal(L, "require");
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 1);
    return;
  }

  lua_pushcclosure(L, requireWithSinenAlias, "require", 1);
  lua_setglobal(L, "require");
}

static void clearSceneEntryPoints(lua_State *L) {
  lua_pushnil(L);
  lua_setglobal(L, "setup");
  lua_pushnil(L);
  lua_setglobal(L, "update");
  lua_pushnil(L);
  lua_setglobal(L, "draw");
}

static void drawNowLoadingOverlay() {
  if (gScenePhase != ScriptScenePhase::Loading) {
    return;
  }

  const uint32_t total = gSetupTasks.total();
  const uint32_t pending = gSetupTasks.pending();
  const float progress =
      (total == 0) ? 1.0f : (static_cast<float>(total - pending) / total);
  const float clampedProgress = std::clamp(progress, 0.0f, 1.0f);

  const Vec2 windowSize = Window::size();
  const float margin = 32.0f;
  const float maxBarWidth = std::max(64.0f, windowSize.x - margin * 2.0f);
  const float barWidth = std::min(480.0f, maxBarWidth);
  const float barHeight = 10.0f;
  const float barX = (windowSize.x - barWidth) * 0.5f;
  const float barY = windowSize.y * 0.82f;

  Graphics::drawRect(Rect(0.0f, 0.0f, windowSize.x, windowSize.y),
                     Color(0.0f, 0.0f, 0.0f, 0.35f));
  Graphics::drawRect(Rect(barX, barY, barWidth, barHeight),
                     Color(1.0f, 1.0f, 1.0f, 0.22f));
  Graphics::drawRect(Rect(barX, barY, barWidth * clampedProgress, barHeight),
                     Color(0.18f, 0.62f, 0.95f, 0.95f));
}

int luaLoadSource(lua_State *L, const String &source, const String &chunkname,
                  StringView fullPath) {
  Luau::CompileOptions options{};
  options.debugLevel = 2;
  std::string bytecode =
      Luau::compile(std::string(source.data(), source.size()), options);
  int status =
      luau_load(L, chunkname.c_str(), bytecode.data(), bytecode.size(), 0);
  if (status == LUA_OK) {
    debugger.onLuaFileLoaded(gLua, fullPath, true);
    return LUA_OK;
  }
  const char *msg = lua_tostring(L, -1);
  Log::error("[luau load error] {}", msg ? msg : "(unknown error)");
  lua_pop(L, 1);
  return status;
}

static int luaLoadBytecode(lua_State *L, const String &bytecode,
                           const String &chunkname, StringView fullPath) {
  int status =
      luau_load(L, chunkname.c_str(), bytecode.data(), bytecode.size(), 0);
  if (status == LUA_OK) {
    debugger.onLuaFileLoaded(gLua, fullPath, true);
    return LUA_OK;
  }
  const char *msg = lua_tostring(L, -1);
  Log::error("[luau load error] {}", msg ? msg : "(unknown error)");
  lua_pop(L, 1);
  return status;
}
void registerVec2(lua_State *);
void registerVec3(lua_State *);
void registerMat4(lua_State *);
void registerColor(lua_State *);
void registerAABB(lua_State *);
void registerRay(lua_State *);
void registerTimer(lua_State *);
void registerCollider(lua_State *);
void registerCamera(lua_State *);
void registerCamera2D(lua_State *);
void registerBuffer(lua_State *);
void registerPivot(lua_State *);
void registerRect(lua_State *);
void registerTransform(lua_State *);
void registerFont(lua_State *);
void registerTextStyle(lua_State *);
void registerTexture(lua_State *);
void registerRenderTexture(lua_State *);
void registerSound(lua_State *);
void registerSynth(lua_State *);
void registerShader(lua_State *);
void registerShaderBundle(lua_State *);
void registerPipeline(lua_State *);
void registerComputeBuffer(lua_State *);
void registerComputePipeline(lua_State *);
void registerCompute(lua_State *);
void registerRaytracing(lua_State *);
void registerModel(lua_State *);
void registerMeshBuilder(lua_State *);
void registerTextureKey(lua_State *);
void registerRandom(lua_State *);
void registerWindow(lua_State *);
void registerPhysics(lua_State *);
void registerShaderStage(lua_State *);
void registerShaderFormat(lua_State *);
void registerBuiltinShader(lua_State *);
void registerBuiltinPipeline(lua_State *);
void registerEvent(lua_State *);
void registerGraphics(lua_State *);
void registerGui(lua_State *);
void registerMouse(lua_State *);
void registerKeyboard(lua_State *);
void registerGamepad(lua_State *);
void registerFilesystem(lua_State *);
void registerScript(lua_State *);
void registerLog(lua_State *);
void registerPeriodic(lua_State *);
void registerTime(lua_State *);
void registerVideo(lua_State *);

static void requireConfigInit(luarequire_Configuration *config) {
  config->is_require_allowed = isRequireAllowed;
  config->reset = reset;
  config->jump_to_alias = jumpToAlias;
  config->to_alias_override = nullptr;
  config->to_alias_fallback = nullptr;
  config->to_parent = toParent;
  config->to_child = toChild;
  config->is_module_present = isModulePresent;
  config->get_chunkname = getChunkname;
  config->get_loadname = getLoadname;
  config->get_cache_key = getCacheKey;
  config->get_config_status = getConfigStatus;
  config->get_alias = nullptr;
  config->get_config = getConfig;
  config->get_luau_config_timeout = getLuauConfigTimeout;
  config->load = load;
}

static void registerAll(lua_State *L) {
  static RequireContext requireCtx{};
  luaopen_require(L, requireConfigInit, &requireCtx);

  registerVec2(L);
  registerVec3(L);
  registerMat4(L);
  registerColor(L);
  registerAABB(L);
  registerRay(L);
  registerTimer(L);
  registerCollider(L);
  registerCamera(L);
  registerCamera2D(L);
  registerBuffer(L);
  registerPivot(L);
  registerRect(L);
  registerTransform(L);

  registerFont(L);
  registerTextStyle(L);
  registerTexture(L);
  registerRenderTexture(L);
  registerSound(L);
  registerSynth(L);
  registerShader(L);
  registerShaderBundle(L);
  registerPipeline(L);
  registerComputeBuffer(L);
  registerComputePipeline(L);
  registerRaytracing(L);
  registerModel(L);
  registerMeshBuilder(L);
  registerTextureKey(L);

  registerRandom(L);
  registerWindow(L);
  registerPhysics(L);
  registerShaderStage(L);
  registerShaderFormat(L);
  registerBuiltinShader(L);
  registerBuiltinPipeline(L);
  registerEvent(L);

  registerGraphics(L);
  registerCompute(L);
  registerGui(L);
  registerMouse(L);
  registerKeyboard(L);
  registerGamepad(L);
  registerFilesystem(L);
  registerScript(L);
  registerLog(L);
  registerPeriodic(L);
  registerTime(L);
  registerVideo(L);
}

bool Script::initialize() {
#ifndef SINEN_NO_USE_SCRIPT
  auto logHandler = [](std::string_view msg) { Log::info("{}", msg.data()); };
  auto errorHandler = [](std::string_view msg) {
    Log::error("{}", msg.data());
  };

  luau::debugger::log::install(logHandler, errorHandler);
  debugger.listen(58000);
  // bindings are implemented using Lua C API (see per-module *lua.cpp files)
  gLua = lua_newstate(alloc, nullptr);
  if (!gLua) {
    Log::error("lua_newstate failed");
    return false;
  }
  luaL_openlibs(gLua);
  debugger.initialize(gLua);
  if (auto *cb = lua_callbacks(gLua)) {
    cb->panic = [](lua_State *L, int errcode) {
      const char *msg = lua_tostring(L, -1);
      Log::critical("[luau panic {}] {}", errcode,
                    msg ? msg : "(unknown error)");
    };
  }

  lua_newtable(gLua);
  lua_setglobal(gLua, "sn");

  registerAll(gLua);
  installRequireAlias(gLua);

  Graphics::addPostDrawFunc(drawNowLoadingOverlay);
#endif
  return true;
}
void Script::shutdown() {
#ifndef SINEN_NO_USE_SCRIPT
  if (!gLua) {
    return;
  }
  if (gSetupRef != LUA_NOREF) {
    luaLUnref2(gLua, LUA_REGISTRYINDEX, gSetupRef);
    gSetupRef = LUA_NOREF;
  }
  if (gUpdateRef != LUA_NOREF) {
    luaLUnref2(gLua, LUA_REGISTRYINDEX, gUpdateRef);
    gUpdateRef = LUA_NOREF;
  }
  if (gDrawRef != LUA_NOREF) {
    luaLUnref2(gLua, LUA_REGISTRYINDEX, gDrawRef);
    gDrawRef = LUA_NOREF;
  }
  lua_gc(gLua, LUA_GCCOLLECT, 0);
  lua_close(gLua);
  gLua = nullptr;
#endif // SINEN_NO_USE_SCRIPT
}

static const char *nothingSceneLua = R"(
local sn = require("@sinen")
local font: sn.Font = sn.Font.new()
font:load(32)

function draw()
	sn.Graphics.drawText(
		"NO DATA",
		sn.TextStyle.new(font, sn.Color.new(1.0), 32),
		sn.TextTransform.new(sn.Window.center(), 0.0, sn.Pivot.Center)
  )
end
)";

static bool startsWithUserScheme(StringView path) {
  constexpr StringView scheme = "user://";
  return path.size() >= scheme.size() &&
         path.substr(0, scheme.size()) == scheme;
}

static bool startsWithLogicalRoot(StringView path) {
  return !path.empty() && (path[0] == '/' || path[0] == '\\');
}

static bool isPathSeparator(char c) { return c == '/' || c == '\\'; }

static bool hasScriptExtension(StringView path) {
  constexpr StringView sourceExt = prefix;
  constexpr StringView bytecodeExt = ".snb";
  return (path.size() >= sourceExt.size() &&
          path.substr(path.size() - sourceExt.size()) == sourceExt) ||
         (path.size() >= bytecodeExt.size() &&
          path.substr(path.size() - bytecodeExt.size()) == bytecodeExt);
}

static String normalizeLogicalPath(StringView path) {
  String scheme;
  StringView rest = path;
  if (startsWithUserScheme(rest)) {
    scheme = "user://";
    rest.remove_prefix(scheme.size());
  } else if (startsWithLogicalRoot(rest)) {
    scheme = "/";
  }

  while (!rest.empty() && startsWithLogicalRoot(rest)) {
    rest.remove_prefix(1);
  }

  Array<String> parts;
  String current;
  for (char c : rest) {
    const char normalized = isPathSeparator(c) ? '/' : c;
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

  String out = scheme;
  for (size_t i = 0; i < parts.size(); ++i) {
    if (!out.empty() && out.back() != '/') {
      out.push_back('/');
    } else if (out.empty() && i > 0) {
      out.push_back('/');
    }
    out += parts[i];
  }

  if (out.empty()) {
    return ".";
  }
  return out;
}

static String joinLogicalPath(StringView base, StringView path) {
  if (startsWithUserScheme(path) || startsWithLogicalRoot(path)) {
    return normalizeLogicalPath(path);
  }

  String combined = base.empty() ? String(".") : String(base);
  if (!combined.empty() && !isPathSeparator(combined.back())) {
    combined.push_back('/');
  }
  combined += String(path);
  return normalizeLogicalPath(combined);
}

static String dirnameLogicalPath(StringView path) {
  if (startsWithUserScheme(path)) {
    constexpr StringView scheme = "user://";
    StringView rest = path.substr(scheme.size());
    const size_t slash = rest.find_last_of("/\\");
    if (slash == StringView::npos) {
      return String(scheme);
    }
    return normalizeLogicalPath(String(scheme) + String(rest.substr(0, slash)));
  }

  const size_t slash = path.find_last_of("/\\");
  if (slash == StringView::npos) {
    return ".";
  }
  if (slash == 0 && startsWithLogicalRoot(path)) {
    return "/";
  }
  return normalizeLogicalPath(path.substr(0, slash));
}

static String basenameLogicalPath(StringView path) {
  if (startsWithUserScheme(path)) {
    constexpr StringView scheme = "user://";
    path.remove_prefix(scheme.size());
  }
  const size_t slash = path.find_last_of("/\\");
  if (slash == StringView::npos) {
    return String(path);
  }
  return String(path.substr(slash + 1));
}

struct ScriptChunk {
  String bytes;
  String filename;
  bool bytecode = false;
};

static ScriptChunk loadSceneChunk(StringView sceneName) {
  ScriptChunk chunk;

  if (hasScriptExtension(sceneName)) {
    chunk.bytes = AssetReader::openAsString(sceneName);
    if (!chunk.bytes.empty()) {
      chunk.filename = sceneName;
      chunk.bytecode = sceneName.size() >= 4 &&
                       sceneName.substr(sceneName.size() - 4) == ".snb";
    }
    return chunk;
  }

  const String bytecodeName = String(sceneName) + ".snb";
  if (AssetReader::exists(bytecodeName)) {
    chunk.bytes = AssetReader::openAsString(bytecodeName);
    if (!chunk.bytes.empty()) {
      chunk.filename = bytecodeName;
      chunk.bytecode = true;
      return chunk;
    }
  }

  const String sourceName = String(sceneName) + prefix;
  chunk.bytes = AssetReader::openAsString(sourceName);
  if (!chunk.bytes.empty()) {
    chunk.filename = sourceName;
  }
  return chunk;
}

void Script::runScene() {
#ifndef SINEN_NO_USE_SCRIPT
  if (!gLua) {
    return;
  }
  lua_settop(gLua, 0);

  gScenePhase = ScriptScenePhase::Running;
  gSetupTasks = TaskGroup::create();
  ScopedLoadContext loadCtx(gSetupTasks);

  auto logPCallError = [](lua_State *L) {
    const char *msg = lua_tostring(L, -1);
    Log::error("[lua error] {}", msg ? msg : "(unknown error)");
    lua_pop(L, 1);
  };

  ScriptChunk chunk = loadSceneChunk(sceneName);
  if (chunk.bytes.empty()) {
    chunk.bytes = nothingSceneLua;
    chunk.filename = String(sceneName) + prefix;
    chunk.bytecode = false;
  }

  if (gSetupRef != LUA_NOREF) {
    luaLUnref2(gLua, LUA_REGISTRYINDEX, gSetupRef);
    gSetupRef = LUA_NOREF;
  }
  if (gUpdateRef != LUA_NOREF) {
    luaLUnref2(gLua, LUA_REGISTRYINDEX, gUpdateRef);
    gUpdateRef = LUA_NOREF;
  }
  if (gDrawRef != LUA_NOREF) {
    luaLUnref2(gLua, LUA_REGISTRYINDEX, gDrawRef);
    gDrawRef = LUA_NOREF;
  }
  clearSceneEntryPoints(gLua);

  String loadPath = AssetReader::getLoadPath(chunk.filename);
  String chunkname = "@" + loadPath;
  auto fullPath = AssetReader::isArchiveMounted()
                      ? loadPath
                      : AssetReader::getFilePath(chunk.filename);
  if (fullPath.empty()) {
    fullPath = chunk.filename;
  }
  const int loadStatus =
      chunk.bytecode
          ? luaLoadBytecode(gLua, chunk.bytes, fullPath.c_str(), fullPath)
          : luaLoadSource(gLua, chunk.bytes, fullPath.c_str(), fullPath);
  if (loadStatus != LUA_OK) {
    lua_settop(gLua, 0);
    return;
  }
  if (lua_pcall(gLua, 0, 0, 0) != LUA_OK) {
    logPCallError(gLua);
    lua_settop(gLua, 0);
    return;
  }

  lua_getglobal(gLua, "setup");
  int funcIndex = LUA_REGISTRYINDEX;
  funcIndex = -1;
  if (lua_isfunction(gLua, -1)) {
    gSetupRef = luaLRef2(gLua, funcIndex);
  } else {
    lua_pop(gLua, 1);
  }
  lua_getglobal(gLua, "update");
  if (lua_isfunction(gLua, -1)) {
    gUpdateRef = luaLRef2(gLua, funcIndex);
  } else {
    lua_pop(gLua, 1);
  }
  lua_getglobal(gLua, "draw");
  if (lua_isfunction(gLua, -1)) {
    gDrawRef = luaLRef2(gLua, funcIndex);
  } else {
    lua_pop(gLua, 1);
  }

  if (gSetupRef != LUA_NOREF) {
    lua_rawgeti(gLua, LUA_REGISTRYINDEX, gSetupRef);
    if (lua_pcall(gLua, 0, 0, 0) != LUA_OK) {
      logPCallError(gLua);
    }
  }
  lua_settop(gLua, 0);

  if (!gSetupTasks.isDone()) {
    gScenePhase = ScriptScenePhase::Loading;
  } else {
    gScenePhase = ScriptScenePhase::Running;
  }
#endif
}

void Script::updateScene() {
#ifndef SINEN_NO_USE_SCRIPT
  if (gScenePhase == ScriptScenePhase::Loading) {
    if (gSetupTasks.isDone()) {
      gScenePhase = ScriptScenePhase::Running;
    } else {
      return;
    }
  }

  if (!gLua || gUpdateRef == LUA_NOREF) {
    return;
  }
  lua_rawgeti(gLua, LUA_REGISTRYINDEX, gUpdateRef);
  if (lua_pcall(gLua, 0, 0, 0) != LUA_OK) {
    const char *msg = lua_tostring(gLua, -1);
    Log::error("[lua error] {}", msg ? msg : "(unknown error)");
    lua_pop(gLua, 1);
  }
#endif
}

void Script::drawScene() {
#ifndef SINEN_NO_USE_SCRIPT
  if (gScenePhase == ScriptScenePhase::Loading) {
    return;
  }
  if (!gLua || gDrawRef == LUA_NOREF) {
    return;
  }
  lua_rawgeti(gLua, LUA_REGISTRYINDEX, gDrawRef);
  if (lua_pcall(gLua, 0, 0, 0) != LUA_OK) {
    const char *msg = lua_tostring(gLua, -1);
    Log::error("[lua error] {}", msg ? msg : "(unknown error)");
    lua_pop(gLua, 1);
  }
#endif
}

bool Script::hasToReload() { return reload; }
void Script::doneReload() { reload = false; }
void Script::clearRequireCache() {
#ifndef SINEN_NO_USE_SCRIPT
  if (!gLua) {
    return;
  }
  luarequire_clearcache(gLua);
  lua_settop(gLua, 0);
#endif
}
void Script::setSceneName(StringView name) {
  sceneName = name;
  reload = true;
}
String Script::getSceneName() { return sceneName; }
void Script::load(StringView filePath) {
  String normalized = joinLogicalPath(basePath, filePath);
  if (!hasScriptExtension(normalized)) {
    Log::error("Script.load requires a .luau or .snb extension: {}",
               normalized.c_str());
    return;
  }
  String nextSceneName = basenameLogicalPath(normalized);
  if (nextSceneName.empty()) {
    nextSceneName = "main";
  }
  sceneName = nextSceneName;
  basePath = dirnameLogicalPath(normalized);
  reload = true;
}
void Script::setBasePath(StringView path) {
  if (!path.empty()) {
    basePath = normalizeLogicalPath(path);
  }
  reload = true;
}
String Script::getBasePath() { return basePath; }
String Script::getRootBasePath() { return rootBasePath; }
} // namespace sinen

namespace sinen {
static int lScriptLoad(lua_State *L) {
  int n = lua_gettop(L);
  if (n != 1) {
    return luaLError2(L, "Script.load expects exactly one path argument");
  }
  const char *filePath = luaL_checkstring(L, 1);
  Script::load(StringView(filePath));
  return 0;
}
void registerScript(lua_State *L) {
  pushSnNamed(L, "Script");
  luaPushcfunction2(L, lScriptLoad);
  lua_setfield(L, -2, "load");
  lua_pop(L, 1);
}
} // namespace sinen
