#include "luaapi.hpp"
#include <gpu/shader/shader_stage.hpp>

namespace sinen {
void registerShaderStage(lua_State *L) {
  pushSnNamed(L, "ShaderStage");
  luau::table(L, -1).value("Vertex",
                           static_cast<lua_Integer>(ShaderStage::Vertex));
  luau::table(L, -1).value("Fragment",
                           static_cast<lua_Integer>(ShaderStage::Fragment));
  luau::table(L, -1).value("Compute",
                           static_cast<lua_Integer>(ShaderStage::Compute));
  luau::table(L, -1).value(
      "RayGeneration", static_cast<lua_Integer>(ShaderStage::RayGeneration));
  luau::table(L, -1).value("AnyHit",
                           static_cast<lua_Integer>(ShaderStage::AnyHit));
  luau::table(L, -1).value("ClosestHit",
                           static_cast<lua_Integer>(ShaderStage::ClosestHit));
  luau::table(L, -1).value("Miss", static_cast<lua_Integer>(ShaderStage::Miss));
  luau::table(L, -1).value("Intersection",
                           static_cast<lua_Integer>(ShaderStage::Intersection));
  luau::table(L, -1).value("Callable",
                           static_cast<lua_Integer>(ShaderStage::Callable));
  lua_pop(L, 1);
}
} // namespace sinen
