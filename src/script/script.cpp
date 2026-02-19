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
#include <math/graph/bfs_grid.hpp>
#include <platform/io/asset_io.hpp>
#include <platform/io/filesystem.hpp>

#include "luaapi.hpp"
#include "require.hpp"
#include <Luau/Require.h>

#include <debugger.h>

#include <imgui.h>

#include <SDL3/SDL.h>

#include <algorithm>
#include <cctype>
#include <filesystem>
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
  luaL_error(L, va_list(ap));
  return 0;
}
int luaLRef2(lua_State *L, int idx) { return lua_ref(L, idx); }
void luaLUnref2(lua_State *L, int idx, int r) { lua_unref(L, r); }
void pushSnNamed(lua_State *L, const char *name) {
  lua_newtable(L);
  lua_pushvalue(L, -1);
  lua_setglobal(L, name);
}
String convert(StringView name, const TablePair &p, bool isReturn) {
  String s;
  s = name.data();
  s += "{ ";
  if (isReturn) {
    s += "\n";
  }
  for (int i = 0; i < p.size(); i++) {
    auto &v = p[i];
    if (isReturn) {
      s += "\t";
    }
    s += v.first + " = " + v.second;
    if (i < p.size() - 1) {
      s += ", ";
    }
    if (isReturn) {
      s += "\n";
    }
  }
  s += " }";
  return s;
}
String toStringTrim(double value) {
  String s(std::format("{}", value));

  auto dot = s.find('.');
  if (dot == String::npos) {
    return s + ".0";
  }

  bool allZero = true;
  for (size_t i = dot + 1; i < s.size(); ++i) {
    if (s[i] != '0') {
      allZero = false;
      break;
    }
  }

  if (allZero) {
    return s.substr(0, dot + 2);
  }

  s.erase(s.find_last_not_of('0') + 1);
  return s;
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

static void drawNowLoadingOverlay() {
  if (gScenePhase != ScriptScenePhase::Loading) {
    return;
  }

  const uint32_t total = gSetupTasks.total();
  const uint32_t pending = gSetupTasks.pending();
  const float progress =
      (total == 0) ? 1.0f : (static_cast<float>(total - pending) / total);

  ImGuiIO &io = ImGui::GetIO();

  ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
  ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_Always);
  ImGui::SetNextWindowBgAlpha(0.0f);

  constexpr ImGuiWindowFlags flags =
      ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNav |
      ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
      ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBringToFrontOnFocus;

  ImGui::Begin("##sn_now_loading", nullptr, flags);
  const ImVec2 center =
      ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
  const float w = 280.0f;
  const float h = 30.0f;

  ImGui::SetCursorPos(ImVec2(center.x - w * 0.5f, center.y - 40.0f));
  ImGui::TextUnformatted("Now Loading...");
  ImGui::SetCursorPos(ImVec2(center.x - w * 0.5f, center.y - 10.0f));
  ImGui::ProgressBar(progress, ImVec2(w, h));
  ImGui::End();
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
  Log::error("[luau load error] %s", msg ? msg : "(unknown error)");
  lua_pop(L, 1);
  return status;
}
void registerVec2(lua_State *);
void registerVec3(lua_State *);
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
void registerGrid(lua_State *);
void registerBFSGrid(lua_State *);
void registerFont(lua_State *);
void registerTexture(lua_State *);
void registerRenderTexture(lua_State *);
void registerSound(lua_State *);
void registerSynth(lua_State *);
void registerShader(lua_State *);
void registerPipeline(lua_State *);
void registerModel(lua_State *);
void registerTextureKey(lua_State *);
void registerArguments(lua_State *);
void registerRandom(lua_State *);
void registerWindow(lua_State *);
void registerPhysics(lua_State *);
void registerShaderStage(lua_State *);
void registerBuiltinShader(lua_State *);
void registerBuiltinPipeline(lua_State *);
void registerEvent(lua_State *);
void registerGraphics(lua_State *);
void registerMouse(lua_State *);
void registerKeyboard(lua_State *);
void registerGamepad(lua_State *);
void registerFilesystem(lua_State *);
void registerScript(lua_State *);
void registerLog(lua_State *);
void registerImGui(lua_State *);
void registerPeriodic(lua_State *);
void registerTime(lua_State *);
void registerFile(lua_State *);

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
  registerGrid(L);
  registerBFSGrid(L);

  registerFont(L);
  registerTexture(L);
  registerRenderTexture(L);
  registerSound(L);
  registerSynth(L);
  registerShader(L);
  registerPipeline(L);
  registerModel(L);
  registerTextureKey(L);

  registerArguments(L);
  registerRandom(L);
  registerWindow(L);
  registerPhysics(L);
  registerShaderStage(L);
  registerBuiltinShader(L);
  registerBuiltinPipeline(L);
  registerEvent(L);

  registerGraphics(L);
  registerMouse(L);
  registerKeyboard(L);
  registerGamepad(L);
  registerFilesystem(L);
  registerScript(L);
  registerLog(L);
  registerImGui(L);
  registerPeriodic(L);
  registerTime(L);
  registerFile(L);
}

bool Script::initialize() {
#ifndef SINEN_NO_USE_SCRIPT
  auto logHandler = [](std::string_view msg) {
    printf("%s", msg.data());
#if defined(__ANDROID__)
    __android_log_print(ANDROID_LOG_INFO, "luaud", "%s", msg.data());
#endif
  };
  auto errorHandler = [](std::string_view msg) {
    fprintf(stderr, "%s", msg.data());
#if defined(__ANDROID__)
    __android_log_print(ANDROID_LOG_ERROR, "luaud", "%s", msg.data());
#endif
  };

  luau::debugger::log::install(logHandler, errorHandler);
  while (!debugger.listen(58000)) {
  }
  Log::info("Luau Debug server started on 58000");

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
      Log::critical("[luau panic %d] %s", errcode,
                    msg ? msg : "(unknown error)");
    };
  }

  lua_newtable(gLua);
  lua_setglobal(gLua, "sn");

  registerAll(gLua);

  Graphics::addImGuiFunction(drawNowLoadingOverlay);
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
local font = Font.new()
font:load(32)
function draw()
    Graphics.drawText("NO DATA", font, Vec2.new(0, 0), Color.new(1.0), 32, 0.0)
end
)";
void Script::runScene() {
#ifndef SINEN_NO_USE_SCRIPT
  if (!gLua) {
    return;
  }

  gScenePhase = ScriptScenePhase::Running;
  gSetupTasks = TaskGroup::create();
  ScopedLoadContext loadCtx(gSetupTasks);

  auto logPCallError = [](lua_State *L) {
    const char *msg = lua_tostring(L, -1);
    Log::error("[lua error] {}", msg ? msg : "(unknown error)");
    lua_pop(L, 1);
  };

  String source;
  source = AssetIO::openAsString(String(sceneName) + prefix);
  if (source.empty()) {
    source = nothingSceneLua;
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

  String filename = String(sceneName) + prefix;
  String chunkname = "@" + AssetIO::getFilePath(filename);
  auto fullPath = std::filesystem::current_path().string() + "\\" +
                  AssetIO::getFilePath(filename).c_str();
  if (luaLoadSource(gLua, source, fullPath.c_str(), fullPath) != LUA_OK) {
    logPCallError(gLua);
    return;
  }
  if (lua_pcall(gLua, 0, 0, 0) != LUA_OK) {
    logPCallError(gLua);
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
void Script::setSceneName(StringView name) {
  sceneName = name;
  reload = true;
}
String Script::getSceneName() { return sceneName; }
void Script::load(StringView filePath, StringView baseDirPath) {
  setSceneName(filePath);
  setBasePath(baseDirPath);
}
void Script::setBasePath(StringView path) {
  if (!path.empty()) {
    basePath = path;
  }
  reload = true;
}
String Script::getBasePath() { return basePath; }
} // namespace sinen
