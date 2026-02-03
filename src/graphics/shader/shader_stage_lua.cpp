#include <graphics/shader/shader_stage.hpp>
#include <script/luaapi.hpp>

namespace sinen {
void registerShaderStage(lua_State *L) {
  pushSnNamed(L, "ShaderStage");
  lua_pushinteger(L, static_cast<lua_Integer>(ShaderStage::Vertex));
  lua_setfield(L, -2, "Vertex");
  lua_pushinteger(L, static_cast<lua_Integer>(ShaderStage::Fragment));
  lua_setfield(L, -2, "Fragment");
  lua_pushinteger(L, static_cast<lua_Integer>(ShaderStage::Compute));
  lua_setfield(L, -2, "Compute");
  lua_pop(L, 1);
}
} // namespace sinen
