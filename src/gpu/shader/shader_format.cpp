#include "shader_format.hpp"
#include <script/luaapi.hpp>

namespace sinen {
void registerShaderFormat(lua_State *L) {
  pushSnNamed(L, "ShaderFormat");
  lua_pushinteger(L, static_cast<lua_Integer>(ShaderFormat::SPIRV));
  lua_setfield(L, -2, "SPIRV");
  lua_pushinteger(L, static_cast<lua_Integer>(ShaderFormat::WGSL));
  lua_setfield(L, -2, "WGSL");
  lua_pop(L, 1);
}
} // namespace sinen
