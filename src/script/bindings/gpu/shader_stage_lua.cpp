#include <gpu/shader/shader_stage.hpp>
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
  lua_pushinteger(L, static_cast<lua_Integer>(ShaderStage::RayGeneration));
  lua_setfield(L, -2, "RayGeneration");
  lua_pushinteger(L, static_cast<lua_Integer>(ShaderStage::AnyHit));
  lua_setfield(L, -2, "AnyHit");
  lua_pushinteger(L, static_cast<lua_Integer>(ShaderStage::ClosestHit));
  lua_setfield(L, -2, "ClosestHit");
  lua_pushinteger(L, static_cast<lua_Integer>(ShaderStage::Miss));
  lua_setfield(L, -2, "Miss");
  lua_pushinteger(L, static_cast<lua_Integer>(ShaderStage::Intersection));
  lua_setfield(L, -2, "Intersection");
  lua_pushinteger(L, static_cast<lua_Integer>(ShaderStage::Callable));
  lua_setfield(L, -2, "Callable");
  lua_pop(L, 1);
}
} // namespace sinen
