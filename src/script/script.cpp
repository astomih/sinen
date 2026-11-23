// internal
#include <script/bindings/external_file_lua.hpp>
#include <script/script.hpp>

#include <core/allocator/global_allocator.hpp>
#include <core/buffer/buffer.hpp>
#include <core/core.hpp>
#include <core/data/ptr.hpp>
#include <core/data/string.hpp>
#include <core/logger/log.hpp>
#include <core/thread/load_context.hpp>
#include <core/thread/task_group.hpp>
#include <gpu/shader/shader_compiler_plugin.h>
#include <graphics/graphics.hpp>
#include <platform/io/asset_reader.hpp>
#include <platform/io/filesystem.hpp>
#include <platform/window/window.hpp>

#include "bindings/luaapi.hpp"
#include "gpu/shader/shader_compiler_service.hpp"
#include "native_module.hpp"
#include "require.hpp"
#include "require_alias.hpp"
#include <Luau/Require.h>
#ifdef SINEN_LUAU_CODEGEN
#include <Luau/CodeGen.h>
#endif

#include <debugger.h>

#include <SDL3/SDL.h>

#include <algorithm>
#include <atomic>
#include <cctype>
#include <cstring>
#include <memory>
#include <optional>
#include <string_view>

namespace sinen {
namespace {
String gFileName = "main.luau";
String gBaseDirectory = "/";
bool reload = true;
std::unique_ptr<::luau::debugger::Debugger> debugger;
std::atomic<bool> debuggerConnected = false;
bool debuggerAttached = false;
} // namespace
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

static lua_State *gLua = nullptr;
lua_State *getGlobalLua() { return gLua; }
static int gSetupRef = LUA_NOREF;
static int gReadyRef = LUA_NOREF;
static int gUpdateRef = LUA_NOREF;
static int gDrawRef = LUA_NOREF;
static int gFileDroppedRef = LUA_NOREF;

enum class ScriptScenePhase {
  Running,
  Loading,
  ReadyPending,
};
static ScriptScenePhase gScenePhase = ScriptScenePhase::Running;
static TaskGroup gSetupTasks;

static void clearSceneEntryPoints(lua_State *L) {
  lua_pushnil(L);
  lua_setglobal(L, "setup");
  lua_pushnil(L);
  lua_setglobal(L, "ready");
  lua_pushnil(L);
  lua_setglobal(L, "update");
  lua_pushnil(L);
  lua_setglobal(L, "draw");
  lua_pushnil(L);
  lua_setglobal(L, "fileDropped");
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

  const Vec2 center = Window::center();
  const float barX = center.x - barWidth * 0.5f;
  const float barY = center.y - barHeight * 0.5f;

  Graphics::drawRect(Rect(0.0f, 0.0f, windowSize.x, windowSize.y),
                     Color(0.0f, 0.0f, 0.0f, 0.35f));
  Graphics::drawRect(Rect(barX, barY, barWidth, barHeight),
                     Color(1.0f, 1.0f, 1.0f, 0.22f));
  Graphics::drawRect(Rect(barX, barY, barWidth * clampedProgress, barHeight),
                     Color(0.18f, 0.62f, 0.95f, 0.95f));
}

static int luaLoadBytecode(lua_State *L, std::string_view bytecode,
                           const String &chunkname, StringView fullPath) {
  int status =
      luau_load(L, chunkname.c_str(), bytecode.data(), bytecode.size(), 0);
  if (status == LUA_OK) {
    if (debuggerAttached) {
      debugger->onLuaFileLoaded(gLua, fullPath, true);
    }
#ifdef SINEN_LUAU_CODEGEN
    if (Luau::CodeGen::isNativeExecutionEnabled(L)) {
      Luau::CodeGen::compile(L, -1, Luau::CodeGen::CompilationOptions{});
    }
#endif
    return LUA_OK;
  }
  const char *msg = lua_tostring(L, -1);
  Log::error("[luau load error] {}", msg ? msg : "(unknown error)");
  lua_pop(L, 1);
  return status;
}

int luaLoadSource(lua_State *L, const String &source, const String &chunkname,
                  StringView fullPath) {
  Luau::CompileOptions options{};
  options.debugLevel = 2;
  std::string bytecode =
      Luau::compile(std::string(source.data(), source.size()), options);
  return luaLoadBytecode(L, bytecode, chunkname, fullPath);
}
void registerVec2(luau::table &);
void registerVec3(luau::table &);
void registerMat4(luau::table &);
void registerColor(luau::table &);
void registerAABB(luau::table &);
void registerRay(luau::table &);
void registerTimer(luau::table &);
void registerCollider(luau::table &);
void registerCamera(luau::table &);
void registerCamera2D(luau::table &);
void registerBuffer(luau::table &);
void registerPivot(luau::table &);
void registerRect(luau::table &);
void registerTransform(luau::table &);
void registerFont(luau::table &);
void registerTextStyle(luau::table &);
void registerTexture(luau::table &);
void registerRenderTexture(luau::table &);
void registerSound(luau::table &);
void registerSynth(luau::table &);
void registerShader(luau::table &);
void registerShaderCompilerPluginBinding(lua_State *);
void registerShaderBundle(luau::table &);
void registerPipeline(luau::table &);
void registerComputeBuffer(luau::table &);
void registerComputePipeline(luau::table &);
void registerCompute(luau::table &);
void registerRaytracing(luau::table &);
void registerModel(luau::table &);
void registerMeshBuilder(luau::table &);
void registerTextureKey(luau::table &);
void registerRandom(luau::table &);
void registerWindow(luau::table &);
void registerPhysics(luau::table &);
void registerShaderStage(luau::table &);
void registerShaderFormat(luau::table &);
void registerBuiltinShader(luau::table &);
void registerBuiltinPipeline(luau::table &);
void registerEvent(luau::table &);
void registerGraphics(luau::table &);
#ifdef SINEN_MODULE_RENDERER
void registerRenderer(luau::table &);
#endif
void registerGui(luau::table &);
void registerMouse(luau::table &);
void registerKeyboard(luau::table &);
void registerGamepad(luau::table &);
void registerFilesystem(luau::table &);
void registerScript(luau::table &);
void registerLog(luau::table &);
void registerPeriodic(luau::table &);
void registerTime(luau::table &);
void registerVideo(luau::table &);

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
  auto sn = luau::state_view(L).get_or_create_table("sn");

  registerVec2(sn);
  registerVec3(sn);
  registerMat4(sn);
  registerColor(sn);
  registerAABB(sn);
  registerRay(sn);
  registerTimer(sn);
  registerCollider(sn);
  registerCamera(sn);
  registerCamera2D(sn);
  registerBuffer(sn);
  registerExternalFile(sn);
  registerPivot(sn);
  registerRect(sn);
  registerTransform(sn);

  registerFont(sn);
  registerTextStyle(sn);
  registerTexture(sn);
  registerRenderTexture(sn);
  registerSound(sn);
  registerSynth(sn);
  registerShader(sn);
  registerShaderBundle(sn);
  registerPipeline(sn);
  registerComputeBuffer(sn);
  registerComputePipeline(sn);
  registerRaytracing(sn);
  registerModel(sn);
  registerMeshBuilder(sn);
  registerTextureKey(sn);

  registerRandom(sn);
  registerWindow(sn);
  registerPhysics(sn);
  registerShaderStage(sn);
  registerShaderFormat(sn);
  registerBuiltinShader(sn);
  registerBuiltinPipeline(sn);
  registerEvent(sn);

  registerGraphics(sn);
#ifdef SINEN_MODULE_RENDERER
  registerRenderer(sn);
#endif
  registerCompute(sn);
  registerGui(sn);
  registerMouse(sn);
  registerKeyboard(sn);
  registerGamepad(sn);
  registerFilesystem(sn);
  registerScript(sn);
  registerLog(sn);
  registerPeriodic(sn);
  registerTime(sn);
  registerVideo(sn);
}

bool Script::initialize(bool isScriptDebug) {
  if (isScriptDebug) {
    debuggerConnected = false;
    auto logHandler = [](std::string_view msg) {
      if (msg.ends_with("] Debugger client connected\n")) {
        debuggerConnected = true;
      }
      Log::info("{}", msg.data());
    };
    auto errorHandler = [](std::string_view msg) {
      Log::error("{}", msg.data());
    };

    ::luau::debugger::log::install(logHandler, errorHandler);
    debugger = std::make_unique<::luau::debugger::Debugger>(false);
    if (debugger->listen(58000)) {
      Log::info("Luau Debug server started on 58000");
      const UInt32 delayStep = 10; // ms
      for (UInt32 elapsed = 0; elapsed < 5000 && !debuggerConnected;
           elapsed += delayStep) {
        SDL_Delay(delayStep);
      }
      if (!debuggerConnected) {
        Log::error("Luau debugger connection timed out.");
        debugger.reset();
      }
    } else {
      debugger.reset();
    }
  }
  // Engine classes and modules are registered through sinen::luau.
  gLua = lua_newstate(alloc, nullptr);
  if (!gLua) {
    debugger.reset();
    Log::error("lua_newstate failed");
    return false;
  }
  luaL_openlibs(gLua);
  if (debugger) {
    debugger->initialize(gLua);
    debuggerAttached = true;
  }
#ifdef SINEN_LUAU_CODEGEN
  // An attached debugger needs the interpreter for stepping and breakpoints.
  if (!debuggerAttached && Luau::CodeGen::isSupported()) {
    Luau::CodeGen::create(gLua);
  }
#endif
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
  installSinenRequireAlias(gLua);

  Graphics::addPostDrawFunc(drawNowLoadingOverlay);
  return true;
}
void Script::shutdown() {
  if (debuggerAttached) {
    debugger->release(gLua);
    debuggerAttached = false;
  }
  debugger.reset();
  if (!gLua) {
    unregisterShaderCompilerPlugin();
    nativeModuleManager().unload();
    return;
  }
  if (gSetupRef != LUA_NOREF) {
    luaLUnref2(gLua, LUA_REGISTRYINDEX, gSetupRef);
    gSetupRef = LUA_NOREF;
  }
  if (gReadyRef != LUA_NOREF) {
    luaLUnref2(gLua, LUA_REGISTRYINDEX, gReadyRef);
    gReadyRef = LUA_NOREF;
  }
  if (gUpdateRef != LUA_NOREF) {
    luaLUnref2(gLua, LUA_REGISTRYINDEX, gUpdateRef);
    gUpdateRef = LUA_NOREF;
  }
  if (gDrawRef != LUA_NOREF) {
    luaLUnref2(gLua, LUA_REGISTRYINDEX, gDrawRef);
    gDrawRef = LUA_NOREF;
  }
  if (gFileDroppedRef != LUA_NOREF) {
    luaLUnref2(gLua, LUA_REGISTRYINDEX, gFileDroppedRef);
    gFileDroppedRef = LUA_NOREF;
  }
  resetExternalFileCallbacks(gLua);
  lua_gc(gLua, LUA_GCCOLLECT, 0);
  lua_gc(gLua, LUA_GCCOLLECT, 0);
  unregisterShaderCompilerPlugin();
  nativeModuleManager().close(gLua);
  lua_close(gLua);
  nativeModuleManager().unload();
  gLua = nullptr;
  gScenePhase = ScriptScenePhase::Running;
  gSetupTasks = TaskGroup();
}

static const char *nothingSceneLua = R"(
local sn = require("@sinen")
local font: sn.Font = sn.Font.new(32)

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
  return (path.size() >= sourceExt.size() &&
          path.substr(path.size() - sourceExt.size()) == sourceExt);
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
};

static ScriptChunk loadSceneChunk(StringView sceneName) {
  ScriptChunk chunk;

  if (hasScriptExtension(sceneName)) {
    chunk.bytes = AssetReader::readAsString(sceneName);
    if (!chunk.bytes.empty()) {
      chunk.filename = sceneName;
    }
    return chunk;
  }

  const String sourceName = String(sceneName) + prefix;
  chunk.bytes = AssetReader::readAsString(sourceName);
  if (!chunk.bytes.empty()) {
    chunk.filename = sourceName;
  }
  return chunk;
}

void Script::executeScene() {
  reload = false;
  if (!gLua) {
    return;
  }
  lua_settop(gLua, 0);
  resetExternalFileCallbacks(gLua);

  gScenePhase = ScriptScenePhase::Running;
  gSetupTasks = TaskGroup::create();
  ScopedLoadContext loadCtx(gSetupTasks);

  auto logPCallError = [](lua_State *L) {
    const char *msg = lua_tostring(L, -1);
    Log::error("[lua error] {}", msg ? msg : "(unknown error)");
    lua_pop(L, 1);
  };

  ScriptChunk chunk = loadSceneChunk(gFileName);
  if (chunk.bytes.empty()) {
    chunk.bytes = nothingSceneLua;
    chunk.filename = String(gFileName) + prefix;
  }

  if (gSetupRef != LUA_NOREF) {
    luaLUnref2(gLua, LUA_REGISTRYINDEX, gSetupRef);
    gSetupRef = LUA_NOREF;
  }
  if (gReadyRef != LUA_NOREF) {
    luaLUnref2(gLua, LUA_REGISTRYINDEX, gReadyRef);
    gReadyRef = LUA_NOREF;
  }
  if (gUpdateRef != LUA_NOREF) {
    luaLUnref2(gLua, LUA_REGISTRYINDEX, gUpdateRef);
    gUpdateRef = LUA_NOREF;
  }
  if (gDrawRef != LUA_NOREF) {
    luaLUnref2(gLua, LUA_REGISTRYINDEX, gDrawRef);
    gDrawRef = LUA_NOREF;
  }
  if (gFileDroppedRef != LUA_NOREF) {
    luaLUnref2(gLua, LUA_REGISTRYINDEX, gFileDroppedRef);
    gFileDroppedRef = LUA_NOREF;
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
      luaLoadSource(gLua, chunk.bytes, fullPath.c_str(), fullPath);
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
  lua_getglobal(gLua, "ready");
  if (lua_isfunction(gLua, -1)) {
    gReadyRef = luaLRef2(gLua, funcIndex);
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
  lua_getglobal(gLua, "fileDropped");
  if (lua_isfunction(gLua, -1)) {
    gFileDroppedRef = luaLRef2(gLua, funcIndex);
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
    gScenePhase = ScriptScenePhase::ReadyPending;
  }
}

static bool prepareSceneFrame() {
  if (gScenePhase == ScriptScenePhase::Loading) {
    if (gSetupTasks.isDone()) {
      gScenePhase = ScriptScenePhase::ReadyPending;
    } else {
      return false;
    }
  }

  if (gScenePhase == ScriptScenePhase::ReadyPending) {
    // Transition first so ready() is never called more than once, even if it
    // reports an error.
    gScenePhase = ScriptScenePhase::Running;
    if (gLua && gReadyRef != LUA_NOREF) {
      lua_rawgeti(gLua, LUA_REGISTRYINDEX, gReadyRef);
      if (lua_pcall(gLua, 0, 0, 0) != LUA_OK) {
        const char *msg = lua_tostring(gLua, -1);
        Log::error("[lua error] {}", msg ? msg : "(unknown error)");
        lua_pop(gLua, 1);
      }
    }
  }

  return gScenePhase == ScriptScenePhase::Running;
}

void Script::callUpdate() {
  if (!prepareSceneFrame()) {
    return;
  }
  if (gLua) {
    pumpExternalFileCallbacks(gLua, gFileDroppedRef);
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
}

void Script::receiveDroppedFile(StringView path) {
  enqueueDroppedExternalFile(path);
}

void Script::callDraw() {
  if (!prepareSceneFrame()) {
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
}

bool Script::hasToReloadScene() { return reload; }
void Script::clearRequireCache() {
  if (!gLua) {
    return;
  }
  luarequire_clearcache(gLua);
  lua_settop(gLua, 0);
}

void Script::load(StringView filePath) {
  String normalized = joinLogicalPath(gBaseDirectory, filePath);
  if (!hasScriptExtension(normalized)) {
    Log::error("Script.load requires a .luau or .sna extension: {}",
               normalized.c_str());
    return;
  }
  gFileName = basenameLogicalPath(normalized);
  gBaseDirectory = dirnameLogicalPath(normalized);
  reload = true;
}
bool Script::load_plugin(StringView pluginName) {
  if (!gLua) {
    nativeModuleManager().loadPlugin(nullptr, pluginName);
    Log::error("Cannot load plugin before Script::initialize: {}",
               nativeModuleManager().lastError());
    return false;
  }
  if (!nativeModuleManager().loadPlugin(gLua, pluginName)) {
    Log::error("Failed to load plugin '{}': {}", pluginName,
               nativeModuleManager().lastError());
    return false;
  }

  void *compilerSymbol = nativeModuleManager().lastPluginSymbol(
      SINEN_SHADER_COMPILER_PLUGIN_API_SYMBOL);
  if (compilerSymbol) {
    sinen_shader_compiler_get_api_fn getCompilerApi = nullptr;
    static_assert(sizeof(getCompilerApi) == sizeof(compilerSymbol));
    std::memcpy(&getCompilerApi, &compilerSymbol, sizeof(getCompilerApi));

    std::string error;
    if (!getCompilerApi ||
        !registerShaderCompilerPlugin(getCompilerApi(), error)) {
      nativeModuleManager().setLastError(std::move(error));
      Log::error("Failed to register shader compiler plugin '{}': {}",
                 pluginName, nativeModuleManager().lastError());
      return false;
    }
    registerShaderCompilerPluginBinding(gLua);
  }
  return true;
}
String Script::getPluginError() {
  const std::string &error = nativeModuleManager().lastError();
  return String(error.data(), error.size());
}
bool Script::loadNativeModule(StringView filePath) {
  return load_plugin(filePath);
}
String Script::getNativeModuleError() { return getPluginError(); }
String Script::getFileName() { return gFileName; }
String Script::getBaseDirectory() { return gBaseDirectory; }
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
static int lScriptClearRequireCache(lua_State *L) {
  if (lua_gettop(L) != 0) {
    return luaLError2(L, "Script.clearRequireCache expects no arguments");
  }
  Script::clearRequireCache();
  return 0;
}
static int lScriptLoadPlugin(lua_State *L) {
  if (lua_gettop(L) != 1) {
    return luaLError2(L, "Script.loadPlugin expects exactly one name argument");
  }
  const char *pluginName = luaL_checkstring(L, 1);
  const bool loaded = Script::load_plugin(StringView(pluginName));
  lua_pushboolean(L, loaded);
  if (loaded) {
    lua_pushnil(L);
  } else {
    String error = Script::getPluginError();
    lua_pushlstring(L, error.data(), error.size());
  }
  return 2;
}
void registerScript(luau::table &sn) {
  auto script = sn.get_or_create_table("Script");
  script.raw_function<lScriptLoad>("load");
  script.raw_function<lScriptClearRequireCache>("clearRequireCache");
  script.raw_function<lScriptLoadPlugin>("loadPlugin");
  script.raw_function<lScriptLoadPlugin>("loadNativeModule");
}
} // namespace sinen
