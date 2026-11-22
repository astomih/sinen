#include "luaapi.hpp"
#include <gpu/shader/shader_format.hpp>

namespace sinen {
void registerShaderFormat(lua_State *L) {
  pushSnNamed(L, "ShaderFormat");
  luau::table(L, -1).value("SPIRV",
                           static_cast<lua_Integer>(ShaderFormat::SPIRV));
  luau::table(L, -1).value("WGSL",
                           static_cast<lua_Integer>(ShaderFormat::WGSL));
  luau::table(L, -1).value("DXBC",
                           static_cast<lua_Integer>(ShaderFormat::DXBC));
  luau::table(L, -1).value("DXIL",
                           static_cast<lua_Integer>(ShaderFormat::DXIL));
  luau::table(L, -1).value("SPIRV_1_3",
                           static_cast<lua_Integer>(ShaderFormat::SPIRV_1_3));
  lua_pop(L, 1);
}
} // namespace sinen
