#include "require.hpp"
#include "luaapi.hpp"
#include "script.hpp"
#include <platform/io/filesystem.hpp>

#include <SDL3/SDL.h>

#include <filesystem>

namespace sinen {
namespace {

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
resolveRequirerFile(sinen::RequireContext &rc, const char *requirer_chunkname) {
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

luarequire_WriteResult writeStringToBuffer(const std::string &value,
                                           char *buffer, size_t buffer_size,
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
bool isRequireAllowed(lua_State *L, void *ctx, const char *requirer_chunkname) {
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
luarequire_NavigateResult reset(lua_State *L, void *ctx,
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
luarequire_NavigateResult jumpToAlias(lua_State *L, void *ctx,
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
luarequire_NavigateResult toAliasOverride(lua_State *L, void *ctx,
                                          const char *alias_unprefixed) {
  (void)L;
  (void)ctx;
  (void)alias_unprefixed;
  return NAVIGATE_NOT_FOUND;
}

// Provides a final opportunity to resolve an alias if it cannot be found in
// configuration files. If NAVIGATE_SUCCESS is returned, the internal state
// must be updated to point at the aliased location. Can be left undefined.
luarequire_NavigateResult toAliasFallback(lua_State *L, void *ctx,
                                          const char *alias_unprefixed) {
  (void)L;
  (void)ctx;
  (void)alias_unprefixed;
  return NAVIGATE_NOT_FOUND;
}

// Navigates through the context by making mutations to the internal state.
luarequire_NavigateResult toParent(lua_State *L, void *ctx) {
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
luarequire_NavigateResult toChild(lua_State *L, void *ctx, const char *name) {
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
bool isModulePresent(lua_State *L, void *ctx) {
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
luarequire_WriteResult getChunkname(lua_State *L, void *ctx, char *buffer,
                                    size_t buffer_size, size_t *size_out) {
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
luarequire_WriteResult getLoadname(lua_State *L, void *ctx, char *buffer,
                                   size_t buffer_size, size_t *size_out) {
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
luarequire_WriteResult getCacheKey(lua_State *L, void *ctx, char *buffer,
                                   size_t buffer_size, size_t *size_out) {
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
luarequire_ConfigStatus getConfigStatus(lua_State *L, void *ctx) {
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
luarequire_WriteResult getAlias(lua_State *L, void *ctx, const char *alias,
                                char *buffer, size_t buffer_size,
                                size_t *size_out) {
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
luarequire_WriteResult getConfig(lua_State *L, void *ctx, char *buffer,
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
int getLuauConfigTimeout(lua_State *L, void *ctx) {
  (void)L;
  (void)ctx;
  return 2000;
}

// Executes the module and places the result on the stack. Returns the
// number of results placed on the stack. Returning -1 directs the requiring
// thread to yield. In this case, this thread should be resumed with the
// module result pushed onto its stack.
int load(lua_State *L, void *ctx, const char *path, const char *chunkname,
         const char *loadname) {
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
                    chunkname ? chunkname : moduleAbs.string().c_str(),
                    moduleAbs.string()) != LUA_OK) {
    // luaLoadSource leaves an error message on the stack.
    return 0;
  }

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
} // namespace sinen