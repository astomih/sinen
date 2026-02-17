#ifndef SINEN_SCRIPT_REQUIRE_HPP
#define SINEN_SCRIPT_REQUIRE_HPP
#include "luaapi.hpp"

#include <Luau/Require.h>

#include <filesystem>
namespace sinen {
struct RequireContext {
  std::filesystem::path root;
  std::filesystem::path current;
};
// Returns whether requires are permitted from the given chunkname.
bool isRequireAllowed(lua_State *L, void *ctx, const char *requirer_chunkname);

// Resets the internal state to point at the requirer module.
luarequire_NavigateResult reset(lua_State *L, void *ctx,
                                const char *requirer_chunkname);
// Resets the internal state to point at an aliased module, given its exact
// path from a configuration file. This function is only called when an
// alias's path cannot be resolved relative to its configuration file.
luarequire_NavigateResult jumpToAlias(lua_State *L, void *ctx,
                                      const char *path);

// Provides an initial alias override opportunity prior to searching for
// configuration files. If NAVIGATE_SUCCESS is returned, the internal state
// must be updated to point at the aliased location. Can be left undefined.
luarequire_NavigateResult toAliasOverride(lua_State *L, void *ctx,
                                          const char *alias_unprefixed);

// Provides a final opportunity to resolve an alias if it cannot be found in
// configuration files. If NAVIGATE_SUCCESS is returned, the internal state
// must be updated to point at the aliased location. Can be left undefined.
luarequire_NavigateResult toAliasFallback(lua_State *L, void *ctx,
                                          const char *alias_unprefixed);
// Navigates through the context by making mutations to the internal state.
luarequire_NavigateResult toParent(lua_State *L, void *ctx);

luarequire_NavigateResult toChild(lua_State *L, void *ctx, const char *name);

// Returns whether the context is currently pointing at a module.
bool isModulePresent(lua_State *L, void *ctx);

// Provides a chunkname for the current module. This will be accessible
// through the debug library. This function is only called if
// is_module_present returns true.
luarequire_WriteResult getChunkname(lua_State *L, void *ctx, char *buffer,
                                    size_t buffer_size, size_t *size_out);

// Provides a loadname that identifies the current module and is passed to
// load. This function is only called if is_module_present returns true.
luarequire_WriteResult getLoadname(lua_State *L, void *ctx, char *buffer,
                                   size_t buffer_size, size_t *size_out);

// Provides a cache key representing the current module. This function is
// only called if is_module_present returns true.
luarequire_WriteResult getCacheKey(lua_State *L, void *ctx, char *buffer,
                                   size_t buffer_size, size_t *size_out);

// Returns whether a configuration file is present in the current context,
// and if so, its syntax. If not present, require-by-string will call
// to_parent until either a configuration file is present or
// NAVIGATE_FAILURE is returned (at root).
luarequire_ConfigStatus getConfigStatus(lua_State *L, void *ctx);

// Parses the configuration file in the current context for the given alias
// and returns its value or WRITE_FAILURE if not found. This function is
// only called if get_config_status returns true. If this function pointer
// is set, get_config must not be set. Opting in to this function pointer
// disables parsing configuration files internally and can be used for finer
// control over the configuration file parsing process.
luarequire_WriteResult getAlias(lua_State *L, void *ctx, const char *alias,
                                char *buffer, size_t buffer_size,
                                size_t *size_out);

// Provides the contents of the configuration file in the current context.
// This function is only called if get_config_status does not return
// CONFIG_ABSENT. If this function pointer is set, get_alias must not be
// set. Opting in to this function pointer enables parsing configuration
// files internally.
luarequire_WriteResult getConfig(lua_State *L, void *ctx, char *buffer,
                                 size_t buffer_size, size_t *size_out);

// Returns the maximum number of milliseconds to allow for executing a given
// Luau-syntax configuration file. This function is only called if
// get_config_status returns CONFIG_PRESENT_LUAU and can be left undefined
// if support for Luau-syntax configuration files is not needed. A default
// value of 2000ms is used. Negative values are treated as infinite.
int getLuauConfigTimeout(lua_State *L, void *ctx);

// Executes the module and places the result on the stack. Returns the
// number of results placed on the stack. Returning -1 directs the requiring
// thread to yield. In this case, this thread should be resumed with the
// module result pushed onto its stack.
int load(lua_State *L, void *ctx, const char *path, const char *chunkname,
         const char *loadname);
} // namespace sinen
#endif // SINEN_SCRIPT_REQUIRE_HPP