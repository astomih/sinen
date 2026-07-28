#include "luaapi.hpp"
#include <gpu/shader/shader_stage.hpp>

namespace sinen {
void registerShaderStage(lua_State *L) {
  pushSnNamed(L, "ShaderStage");
  Binding::registerInteger(L, "Vertex",
                         static_cast<lua_Integer>(ShaderStage::Vertex));
  Binding::registerInteger(L, "Fragment",
                         static_cast<lua_Integer>(ShaderStage::Fragment));
  Binding::registerInteger(L, "Compute",
                         static_cast<lua_Integer>(ShaderStage::Compute));
  Binding::registerInteger(L, "RayGeneration",
                         static_cast<lua_Integer>(ShaderStage::RayGeneration));
  Binding::registerInteger(L, "AnyHit",
                         static_cast<lua_Integer>(ShaderStage::AnyHit));
  Binding::registerInteger(L, "ClosestHit",
                         static_cast<lua_Integer>(ShaderStage::ClosestHit));
  Binding::registerInteger(L, "Miss",
                         static_cast<lua_Integer>(ShaderStage::Miss));
  Binding::registerInteger(L, "Intersection",
                         static_cast<lua_Integer>(ShaderStage::Intersection));
  Binding::registerInteger(L, "Callable",
                         static_cast<lua_Integer>(ShaderStage::Callable));
  lua_pop(L, 1);
}
} // namespace sinen
