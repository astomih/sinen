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

#include <debugger.h>

#include "luaapi.hpp"
#include <Luau/Require.h>

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
  // ensureSn(L);
  // lua_newtable(L);
  // lua_pushvalue(L, -1);
  // lua_setfield(L, -3, name);
  // lua_remove(L, -2); // remove sn
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

static int luaLoadSource(lua_State *L, const String &source,
                         const String &chunkname) {
  Luau::CompileOptions options{};
  options.debugLevel = 2;
  std::string bytecode =
      Luau::compile(std::string(source.data(), source.size()), options);
  int status =
      luau_load(L, chunkname.c_str(), bytecode.data(), bytecode.size(), 0);
  if (status == LUA_OK) {
    return LUA_OK;
  }
  const char *msg = lua_tostring(L, -1);
  Log::error("[luau load error] %s", msg ? msg : "(unknown error)");
  lua_pop(L, 1);
  return status;
}
static constexpr const char *prefix = ".luau";
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

luau::debugger::Debugger debugger(false);
namespace {
struct RequireContext {
  std::filesystem::path root;
  std::filesystem::path current;
};

static bool iequalsAscii(char a, char b) {
  return static_cast<unsigned char>(
             std::tolower(static_cast<unsigned char>(a))) ==
         static_cast<unsigned char>(
             std::tolower(static_cast<unsigned char>(b)));
}

static bool endsWithIcaseAscii(const std::string_view s,
                               const std::string_view suffix) {
  if (suffix.size() > s.size()) {
    return false;
  }
  const size_t offset = s.size() - suffix.size();
  for (size_t i = 0; i < suffix.size(); ++i) {
    if (!iequalsAscii(s[offset + i], suffix[i])) {
      return false;
    }
  }
  return true;
}

static std::filesystem::path getRequireRoot() {
  std::filesystem::path base(Filesystem::getAppBaseDirectory().c_str());
  std::filesystem::path basePath(Script::getBasePath().c_str());

  std::error_code ec;
  std::filesystem::path root = (base / basePath).lexically_normal();
  std::filesystem::path absRoot = std::filesystem::weakly_canonical(root, ec);
  if (ec) {
    absRoot = std::filesystem::absolute(root, ec);
    if (ec) {
      absRoot = root;
    }
  }
  return absRoot.lexically_normal();
}

static std::filesystem::path normalizeChunknamePath(const char *chunkname) {
  if (!chunkname) {
    return {};
  }

  std::string_view s(chunkname);
  if (!s.empty() && s[0] == '@') {
    s.remove_prefix(1);
  }

  // Luau may use special markers like "=[C]" or "=..." for non-file sources.
  if (!s.empty() && s[0] == '=') {
    return {};
  }

  std::string tmp(s);
  std::replace(tmp.begin(), tmp.end(), '\\', '/');
  return std::filesystem::path(tmp).lexically_normal();
}

static bool isPathUnderRootIcase(const std::filesystem::path &path,
                                 const std::filesystem::path &root) {
  auto p = path.lexically_normal();
  auto r = root.lexically_normal();

  auto pit = p.begin();
  auto rit = r.begin();

  for (; rit != r.end(); ++rit, ++pit) {
    if (pit == p.end()) {
      return false;
    }

    const std::string a = pit->generic_string();
    const std::string b = rit->generic_string();
    if (a.size() != b.size()) {
      return false;
    }
    for (size_t i = 0; i < a.size(); ++i) {
      if (!iequalsAscii(a[i], b[i])) {
        return false;
      }
    }
  }

  return true;
}

static std::optional<std::filesystem::path>
resolveExistingFilePath(const std::filesystem::path &p) {
  std::error_code ec;
  if (std::filesystem::exists(p, ec) &&
      std::filesystem::is_regular_file(p, ec)) {
    return p;
  }
  return std::nullopt;
}

static std::optional<std::filesystem::path>
resolveRequirerFile(RequireContext &rc, const char *requirer_chunkname) {
  std::filesystem::path p = normalizeChunknamePath(requirer_chunkname);
  if (p.empty()) {
    return std::nullopt;
  }

  std::error_code ec;
  std::filesystem::path abs = p;
  if (abs.is_relative()) {
    abs = std::filesystem::absolute(abs, ec);
    if (ec) {
      abs = p;
    }
  }

  abs = abs.lexically_normal();
  if (auto hit = resolveExistingFilePath(abs)) {
    return hit;
  }

  // Fallback: `Script::runScene` currently uses a chunkname that may not match
  // the actual loaded asset path. Try resolving by filename inside our root.
  std::filesystem::path filename = abs.filename();
  if (filename.empty()) {
    return std::nullopt;
  }

  std::filesystem::path fallback = (rc.root / filename).lexically_normal();
  if (auto hit = resolveExistingFilePath(fallback)) {
    return hit;
  }

  return std::nullopt;
}

static std::optional<std::filesystem::path>
resolveModuleFileRel(const RequireContext &rc) {
  std::error_code ec;
  const std::filesystem::path fileRel = rc.current;

  std::filesystem::path fileLuau = (fileRel.string() + prefix);
  std::filesystem::path fileAbs = (rc.root / fileLuau).lexically_normal();
  if (std::filesystem::exists(fileAbs, ec) &&
      std::filesystem::is_regular_file(fileAbs, ec)) {
    return fileLuau;
  }

  const String initName = String("init") + prefix;
  std::filesystem::path initRel = fileRel / initName.c_str();
  std::filesystem::path initAbs = (rc.root / initRel).lexically_normal();
  if (std::filesystem::exists(initAbs, ec) &&
      std::filesystem::is_regular_file(initAbs, ec)) {
    return initRel;
  }

  return std::nullopt;
}

static luarequire_WriteResult writeStringToBuffer(const std::string &value,
                                                  char *buffer,
                                                  size_t buffer_size,
                                                  size_t *size_out) {
  if (!size_out) {
    return WRITE_FAILURE;
  }
  *size_out = value.size();
  if (buffer_size < value.size()) {
    return WRITE_BUFFER_TOO_SMALL;
  }
  if (!buffer && !value.empty()) {
    return WRITE_FAILURE;
  }
  if (!value.empty()) {
    std::memcpy(buffer, value.data(), value.size());
  }
  return WRITE_SUCCESS;
}
} // namespace

// Returns whether requires are permitted from the given chunkname.
static bool is_require_allowed(lua_State *L, void *ctx,
                               const char *requirer_chunkname) {
  (void)L;
  (void)ctx;
  // Disallow require from non-file chunks.
  if (!requirer_chunkname || requirer_chunkname[0] == '=') {
    return false;
  }

  std::string_view s(requirer_chunkname);
  if (!s.empty() && s[0] == '@') {
    s.remove_prefix(1);
  }

  // Allow if it looks like a file-backed chunk. We keep this permissive and
  // rely on `reset` to validate and locate the module.
  return endsWithIcaseAscii(s, ".luau") || endsWithIcaseAscii(s, ".lua");
}

// Resets the internal state to point at the requirer module.
static luarequire_NavigateResult reset(lua_State *L, void *ctx,
                                       const char *requirer_chunkname) {
  (void)L;
  auto *rc = static_cast<RequireContext *>(ctx);
  if (!rc) {
    return NAVIGATE_NOT_FOUND;
  }

  rc->root = getRequireRoot();

  auto fileAbsOpt = resolveRequirerFile(*rc, requirer_chunkname);
  if (!fileAbsOpt) {
    return NAVIGATE_NOT_FOUND;
  }
  const std::filesystem::path fileAbs =
      std::filesystem::weakly_canonical(*fileAbsOpt).lexically_normal();

  // Reject chunks outside the configured script root.
  if (!isPathUnderRootIcase(fileAbs, rc->root)) {
    return NAVIGATE_NOT_FOUND;
  }

  std::filesystem::path rel = fileAbs.lexically_relative(rc->root);
  if (rel.empty() || rel.generic_string().starts_with("..")) {
    return NAVIGATE_NOT_FOUND;
  }

  // Convert "foo.luau" -> "foo". If this is an init module, treat the module
  // as the directory that contains it (i.e. "dir/init.luau" -> "dir").
  if (rel.filename() == (String("init") + prefix).c_str()) {
    rc->current = rel.parent_path().lexically_normal();
  } else {
    rel.replace_extension();
    rc->current = rel.lexically_normal();
  }

  return NAVIGATE_SUCCESS;
}

// Resets the internal state to point at an aliased module, given its exact
// path from a configuration file. This function is only called when an
// alias's path cannot be resolved relative to its configuration file.
static luarequire_NavigateResult jump_to_alias(lua_State *L, void *ctx,
                                               const char *path) {
  (void)L;
  auto *rc = static_cast<RequireContext *>(ctx);
  if (!rc || !path) {
    return NAVIGATE_NOT_FOUND;
  }

  std::filesystem::path p(path);
  p = p.lexically_normal();

  // Alias paths are treated as relative to the current script root unless
  // explicitly absolute.
  std::filesystem::path abs = p.is_absolute() ? p : (rc->root / p);

  std::error_code ec;
  if (std::filesystem::is_regular_file(abs, ec)) {
    std::filesystem::path rel = abs.lexically_relative(rc->root);
    if (rel.empty() || rel.generic_string().starts_with("..")) {
      return NAVIGATE_NOT_FOUND;
    }

    if (rel.filename() == (String("init") + prefix).c_str()) {
      rc->current = rel.parent_path().lexically_normal();
    } else {
      rel.replace_extension();
      rc->current = rel.lexically_normal();
    }

    return NAVIGATE_SUCCESS;
  }

  // Accept module identifiers without an extension.
  std::filesystem::path relNoExt = p;
  if (relNoExt.has_extension()) {
    relNoExt.replace_extension();
  }

  std::filesystem::path tryFile = (rc->root / (relNoExt.string() + prefix));
  std::filesystem::path tryDirInit =
      (rc->root / relNoExt / (String("init") + prefix).c_str());

  const bool fileExists = std::filesystem::is_regular_file(tryFile, ec);
  const bool initExists = std::filesystem::is_regular_file(tryDirInit, ec);
  if (fileExists && initExists) {
    return NAVIGATE_AMBIGUOUS;
  }
  if (fileExists || initExists) {
    rc->current = relNoExt.lexically_normal();
    return NAVIGATE_SUCCESS;
  }

  return NAVIGATE_NOT_FOUND;
}

// Provides an initial alias override opportunity prior to searching for
// configuration files. If NAVIGATE_SUCCESS is returned, the internal state
// must be updated to point at the aliased location. Can be left undefined.
static luarequire_NavigateResult
to_alias_override(lua_State *L, void *ctx, const char *alias_unprefixed) {
  (void)L;
  (void)ctx;
  (void)alias_unprefixed;
  return NAVIGATE_NOT_FOUND;
}

// Provides a final opportunity to resolve an alias if it cannot be found in
// configuration files. If NAVIGATE_SUCCESS is returned, the internal state
// must be updated to point at the aliased location. Can be left undefined.
static luarequire_NavigateResult
to_alias_fallback(lua_State *L, void *ctx, const char *alias_unprefixed) {
  (void)L;
  (void)ctx;
  (void)alias_unprefixed;
  return NAVIGATE_NOT_FOUND;
}

// Navigates through the context by making mutations to the internal state.
static luarequire_NavigateResult to_parent(lua_State *L, void *ctx) {
  (void)L;
  auto *rc = static_cast<RequireContext *>(ctx);
  if (!rc) {
    return NAVIGATE_NOT_FOUND;
  }

  if (rc->current.empty()) {
    return NAVIGATE_NOT_FOUND;
  }

  rc->current = rc->current.parent_path().lexically_normal();
  return NAVIGATE_SUCCESS;
}
static luarequire_NavigateResult to_child(lua_State *L, void *ctx,
                                          const char *name) {
  (void)L;
  auto *rc = static_cast<RequireContext *>(ctx);
  if (!rc || !name) {
    return NAVIGATE_NOT_FOUND;
  }

  std::string component(name);
  if (endsWithIcaseAscii(component, prefix)) {
    component.erase(component.size() - std::string(prefix).size());
  }

  std::filesystem::path child = (rc->current / component).lexically_normal();

  std::error_code ec;
  const std::filesystem::path fileAbs = (rc->root / (child.string() + prefix));
  const std::filesystem::path dirAbs = (rc->root / child);

  const bool fileExists = std::filesystem::is_regular_file(fileAbs, ec);
  const bool dirExists = std::filesystem::is_directory(dirAbs, ec);

  if (fileExists && dirExists) {
    return NAVIGATE_AMBIGUOUS;
  }
  if (!fileExists && !dirExists) {
    return NAVIGATE_NOT_FOUND;
  }

  rc->current = child;
  return NAVIGATE_SUCCESS;
}

// Returns whether the context is currently pointing at a module.
static bool is_module_present(lua_State *L, void *ctx) {
  (void)L;
  auto *rc = static_cast<RequireContext *>(ctx);
  if (!rc) {
    return false;
  }
  return resolveModuleFileRel(*rc).has_value();
}

// Provides a chunkname for the current module. This will be accessible
// through the debug library. This function is only called if
// is_module_present returns true.
static luarequire_WriteResult get_chunkname(lua_State *L, void *ctx,
                                            char *buffer, size_t buffer_size,
                                            size_t *size_out) {
  (void)L;
  auto *rc = static_cast<RequireContext *>(ctx);
  if (!rc) {
    return WRITE_FAILURE;
  }
  auto relOpt = resolveModuleFileRel(*rc);
  if (!relOpt) {
    return WRITE_FAILURE;
  }

  std::filesystem::path abs = (rc->root / *relOpt).lexically_normal();
  std::string chunk = "@" + abs.string();
  return writeStringToBuffer(chunk, buffer, buffer_size, size_out);
}

// Provides a loadname that identifies the current module and is passed to
// load. This function is only called if is_module_present returns true.
static luarequire_WriteResult get_loadname(lua_State *L, void *ctx,
                                           char *buffer, size_t buffer_size,
                                           size_t *size_out) {
  (void)L;
  auto *rc = static_cast<RequireContext *>(ctx);
  if (!rc) {
    return WRITE_FAILURE;
  }
  auto relOpt = resolveModuleFileRel(*rc);
  if (!relOpt) {
    return WRITE_FAILURE;
  }

  std::filesystem::path abs = (rc->root / *relOpt).lexically_normal();
  std::string loadname = abs.string();
  return writeStringToBuffer(loadname, buffer, buffer_size, size_out);
}

// Provides a cache key representing the current module. This function is
// only called if is_module_present returns true.
static luarequire_WriteResult get_cache_key(lua_State *L, void *ctx,
                                            char *buffer, size_t buffer_size,
                                            size_t *size_out) {
  (void)L;
  auto *rc = static_cast<RequireContext *>(ctx);
  if (!rc) {
    return WRITE_FAILURE;
  }
  auto relOpt = resolveModuleFileRel(*rc);
  if (!relOpt) {
    return WRITE_FAILURE;
  }

  std::filesystem::path abs = (rc->root / *relOpt).lexically_normal();
  std::string key = abs.generic_string();
  std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });
  return writeStringToBuffer(key, buffer, buffer_size, size_out);
}

// Returns whether a configuration file is present in the current context,
// and if so, its syntax. If not present, require-by-string will call
// to_parent until either a configuration file is present or
// NAVIGATE_FAILURE is returned (at root).
static luarequire_ConfigStatus get_config_status(lua_State *L, void *ctx) {
  (void)L;
  auto *rc = static_cast<RequireContext *>(ctx);
  if (!rc) {
    return CONFIG_ABSENT;
  }

  std::filesystem::path dirAbs = (rc->root / rc->current).lexically_normal();

  std::error_code ec;
  if (!std::filesystem::is_directory(dirAbs, ec)) {
    dirAbs = dirAbs.parent_path();
  }

  const std::filesystem::path jsonAbs = dirAbs / ".luaurc";
  const std::filesystem::path luauAbs = dirAbs / ".config.luau";

  const bool hasJson = std::filesystem::is_regular_file(jsonAbs, ec);
  const bool hasLuau = std::filesystem::is_regular_file(luauAbs, ec);

  if (hasJson && hasLuau) {
    return CONFIG_AMBIGUOUS;
  }
  if (hasJson) {
    return CONFIG_PRESENT_JSON;
  }
  if (hasLuau) {
    return CONFIG_PRESENT_LUAU;
  }

  return CONFIG_ABSENT;
}

// Parses the configuration file in the current context for the given alias
// and returns its value or WRITE_FAILURE if not found. This function is
// only called if get_config_status returns true. If this function pointer
// is set, get_config must not be set. Opting in to this function pointer
// disables parsing configuration files internally and can be used for finer
// control over the configuration file parsing process.
static luarequire_WriteResult get_alias(lua_State *L, void *ctx,
                                        const char *alias, char *buffer,
                                        size_t buffer_size, size_t *size_out) {
  (void)L;
  (void)ctx;
  (void)alias;
  (void)buffer;
  (void)buffer_size;
  (void)size_out;
  return WRITE_FAILURE;
}

// Provides the contents of the configuration file in the current context.
// This function is only called if get_config_status does not return
// CONFIG_ABSENT. If this function pointer is set, get_alias must not be
// set. Opting in to this function pointer enables parsing configuration
// files internally.
static luarequire_WriteResult get_config(lua_State *L, void *ctx, char *buffer,
                                         size_t buffer_size, size_t *size_out) {
  (void)L;
  auto *rc = static_cast<RequireContext *>(ctx);
  if (!rc) {
    return WRITE_FAILURE;
  }

  std::filesystem::path dirAbs = (rc->root / rc->current).lexically_normal();

  std::error_code ec;
  if (!std::filesystem::is_directory(dirAbs, ec)) {
    dirAbs = dirAbs.parent_path();
  }

  const std::filesystem::path jsonAbs = (dirAbs / ".luaurc").lexically_normal();
  const std::filesystem::path luauAbs =
      (dirAbs / ".config.luau").lexically_normal();

  const bool hasJson = std::filesystem::is_regular_file(jsonAbs, ec);
  const bool hasLuau = std::filesystem::is_regular_file(luauAbs, ec);
  if (hasJson && hasLuau) {
    return WRITE_FAILURE;
  }

  std::filesystem::path cfgAbs = hasJson ? jsonAbs : luauAbs;
  if (!std::filesystem::is_regular_file(cfgAbs, ec)) {
    return WRITE_FAILURE;
  }

  // Read directly from disk so we don't depend on Script::getBasePath while
  // navigating across parents.
  auto *file = SDL_IOFromFile(cfgAbs.string().c_str(), "r");
  if (!file) {
    return WRITE_FAILURE;
  }
  size_t fileLength = 0;
  void *load = SDL_LoadFile_IO(file, &fileLength, 1);
  if (!load) {
    return WRITE_FAILURE;
  }

  const std::string contents(reinterpret_cast<const char *>(load), fileLength);
  SDL_free(load);
  return writeStringToBuffer(contents, buffer, buffer_size, size_out);
}

// Returns the maximum number of milliseconds to allow for executing a given
// Luau-syntax configuration file. This function is only called if
// get_config_status returns CONFIG_PRESENT_LUAU and can be left undefined
// if support for Luau-syntax configuration files is not needed. A default
// value of 2000ms is used. Negative values are treated as infinite.
static int get_luau_config_timeout(lua_State *L, void *ctx) {
  (void)L;
  (void)ctx;
  return 2000;
}

// Executes the module and places the result on the stack. Returns the
// number of results placed on the stack. Returning -1 directs the requiring
// thread to yield. In this case, this thread should be resumed with the
// module result pushed onto its stack.
static int load(lua_State *L, void *ctx, const char *path,
                const char *chunkname, const char *loadname) {
  (void)loadname;
  auto *rc = static_cast<RequireContext *>(ctx);
  if (!rc) {
    lua_pushstring(L, "require: missing context");
    return 0;
  }

  auto relOpt = resolveModuleFileRel(*rc);
  if (!relOpt) {
    lua_pushfstring(L, "require: no module present at resolved path (%s)",
                    path ? path : "(unknown)");
    return 0;
  }

  const std::filesystem::path moduleAbs =
      (rc->root / *relOpt).lexically_normal();

  auto *file = SDL_IOFromFile(moduleAbs.string().c_str(), "r");
  if (!file) {
    lua_pushfstring(L, "require: failed to open module file (%s)",
                    moduleAbs.string().c_str());
    return 0;
  }
  size_t fileLength = 0;
  void *raw = SDL_LoadFile_IO(file, &fileLength, 1);
  if (!raw) {
    lua_pushfstring(L, "require: failed to read module file (%s)",
                    moduleAbs.string().c_str());
    return 0;
  }

  const String source(reinterpret_cast<const char *>(raw), fileLength);
  SDL_free(raw);

  const int stackBefore = lua_gettop(L);
  if (luaLoadSource(L, source,
                    chunkname ? chunkname : moduleAbs.string().c_str()) !=
      LUA_OK) {
    // luaLoadSource leaves an error message on the stack.
    return 0;
  }

  debugger.onLuaFileLoaded(gLua, moduleAbs.string(), true);
  // Execute the compiled chunk.
  if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK) {
    // Error message is already on the stack.
    return 0;
  }

  int numResults = lua_gettop(L) - stackBefore;
  if (numResults == 0) {
    // Match Lua's `require`: modules that don't return a value become `true`.
    lua_pushboolean(L, 1);
    numResults = 1;
  }

  return numResults;
}

static void requireConfigInit(luarequire_Configuration *config) {
  config->is_require_allowed = is_require_allowed;
  config->reset = reset;
  config->jump_to_alias = jump_to_alias;
  config->to_alias_override = nullptr;
  config->to_alias_fallback = nullptr;
  config->to_parent = to_parent;
  config->to_child = to_child;
  config->is_module_present = is_module_present;
  config->get_chunkname = get_chunkname;
  config->get_loadname = get_loadname;
  config->get_cache_key = get_cache_key;
  config->get_config_status = get_config_status;
  config->get_alias = nullptr;
  config->get_config = get_config;
  config->get_luau_config_timeout = get_luau_config_timeout;
  config->load = load;
}

static void registerAll(lua_State *L) {

  // luaPushcfunction2(L, Luau::Require::lua_require);
  // lua_setglobal(L, "require");
  static RequireContext requireCtx{};
  luaopen_require(L, requireConfigInit, &requireCtx);

  // // Luau's require-by-string expects "./", "../", or "@" prefixes. For
  // // convenience, treat bare module names as "./<name>" so existing scripts
  // can
  // // write `require("foo")`.
  // {
  //   const String wrapper = R"(
  //     local __sn_require_impl = require
  //     function require(path)
  //       if type(path) == "string" then
  //         if path:sub(1, 2) ~= "./" and path:sub(1, 3) ~= "../" and
  //         path:sub(1, 1) ~= "@" then
  //           path = "./" .. path
  //         end
  //       end
  //       return __sn_require_impl(path)
  //     end
  //   )";
  //
  //   if (luaLoadSource(L, wrapper, "=@sinen_require_wrapper") == LUA_OK) {
  //     if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
  //       const char *msg = lua_tostring(L, -1);
  //       LogF::error("[luau require wrapper error] {}", msg ? msg :
  //       "(unknown)"); lua_pop(L, 1);
  //     }
  //   } else {
  //     const char *msg = lua_tostring(L, -1);
  //     LogF::error("[luau require wrapper compile error] {}",
  //                 msg ? msg : "(unknown)");
  //     lua_pop(L, 1);
  //   }
  // }

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
  if (luaLoadSource(gLua, source, fullPath.c_str()) != LUA_OK) {
    logPCallError(gLua);
    return;
  }
  debugger.onLuaFileLoaded(gLua, fullPath, false);
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

} // namespace sinen
