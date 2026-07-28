#include "luaapi.hpp"
#include <gpu/shader/shader_format.hpp>

namespace sinen {
void registerShaderFormat(lua_State *L) {
  pushSnNamed(L, "ShaderFormat");
  Binding::registerInteger(L, "SPIRV",
                         static_cast<lua_Integer>(ShaderFormat::SPIRV));
  Binding::registerInteger(L, "WGSL",
                         static_cast<lua_Integer>(ShaderFormat::WGSL));
  Binding::registerInteger(L, "DXBC",
                         static_cast<lua_Integer>(ShaderFormat::DXBC));
  Binding::registerInteger(L, "DXIL",
                         static_cast<lua_Integer>(ShaderFormat::DXIL));
  Binding::registerInteger(L, "SPIRV_1_3",
                         static_cast<lua_Integer>(ShaderFormat::SPIRV_1_3));
  lua_pop(L, 1);
}
} // namespace sinen
