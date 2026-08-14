#ifndef SINEN_EXTERNAL_FILE_LUA_HPP
#define SINEN_EXTERNAL_FILE_LUA_HPP

#include <core/data/string.hpp>

struct lua_State;

namespace sinen {

void registerExternalFile(lua_State *L);
void resetExternalFileCallbacks(lua_State *L);
void pumpExternalFileCallbacks(lua_State *L, int fileDroppedRef);
void enqueueDroppedExternalFile(StringView path);

} // namespace sinen

#endif // SINEN_EXTERNAL_FILE_LUA_HPP
