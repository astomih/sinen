#ifndef SINEN_PLUGIN_NATIVEFS_HPP
#define SINEN_PLUGIN_NATIVEFS_HPP

struct lua_State;

namespace sinen::plugins::nativefs {

int open(lua_State *state);
int close(lua_State *state);

} // namespace sinen::plugins::nativefs

#endif // SINEN_PLUGIN_NATIVEFS_HPP
