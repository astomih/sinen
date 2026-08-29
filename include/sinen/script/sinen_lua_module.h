#ifndef SINEN_LUA_MODULE_H
#define SINEN_LUA_MODULE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct lua_State lua_State;

#define SINEN_LUA_MODULE_API_VERSION 1u

#define SINEN_LUA_MODULE_VERSION_SYMBOL "sinen_lua_module_get_api_version"
#define SINEN_LUA_MODULE_OPEN_SYMBOL "sinen_lua_module_open"
#define SINEN_LUA_MODULE_CLOSE_SYMBOL "sinen_lua_module_close"

/* Plugin modules exposed to Luau are stored in the registry under this
 * prefix. A plugin named "example" should register its module table using
 * SINEN_LUA_PLUGIN_REGISTRY_PREFIX "example", then scripts can retrieve it
 * with require("@sinen/plugin/example"). */
#define SINEN_LUA_PLUGIN_REQUIRE_PREFIX "@sinen/plugin/"
#define SINEN_LUA_PLUGIN_REGISTRY_PREFIX "sinen.plugin."

#if defined(_WIN32)
#define SINEN_LUA_MODULE_EXPORT __declspec(dllexport)
#elif defined(__GNUC__) || defined(__clang__)
#define SINEN_LUA_MODULE_EXPORT __attribute__((used, visibility("default")))
#else
#define SINEN_LUA_MODULE_EXPORT
#endif

typedef uint32_t (*sinen_lua_module_get_api_version_fn)(void);
typedef int (*sinen_lua_module_open_fn)(lua_State *state);
typedef int (*sinen_lua_module_close_fn)(lua_State *state);

/* Every module must export these two functions. open is invoked as a protected
 * Lua C function; it should register its functions/classes and return zero.
 */
SINEN_LUA_MODULE_EXPORT uint32_t sinen_lua_module_get_api_version(void);
SINEN_LUA_MODULE_EXPORT int sinen_lua_module_open(lua_State *state);

/* Optional. If exported, close is called while the Lua state is still valid. */
SINEN_LUA_MODULE_EXPORT int sinen_lua_module_close(lua_State *state);

#ifdef __cplusplus
}
#endif

#endif /* SINEN_LUA_MODULE_H */
