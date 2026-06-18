#include "luaapi.hpp"
#include <gpu/shader/shader_format.hpp>

namespace sinen {
void registerShaderFormat(lua_State *L) {
  pushSnNamed(L, "ShaderFormat");
  lua_pushinteger(L, static_cast<lua_Integer>(ShaderFormat::SPIRV));
  lua_setfield(L, -2, "SPIRV");
  lua_pushinteger(L, static_cast<lua_Integer>(ShaderFormat::WGSL));
  lua_setfield(L, -2, "WGSL");
  lua_pushinteger(L, static_cast<lua_Integer>(ShaderFormat::DXBC));
  lua_setfield(L, -2, "DXBC");
  lua_pushinteger(L, static_cast<lua_Integer>(ShaderFormat::DXIL));
  lua_setfield(L, -2, "DXIL");
  lua_pushinteger(L, static_cast<lua_Integer>(ShaderFormat::SPIRV_1_3));
  lua_setfield(L, -2, "SPIRV_1_3");
  lua_pop(L, 1);
}
} // namespace sinen
