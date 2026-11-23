#include "luaapi.hpp"
#include <gpu/shader/shader_format.hpp>

namespace sinen {
void registerShaderFormat(luau::table &sn) {
  auto shaderFormat = sn.get_or_create_table("ShaderFormat");
  shaderFormat.value("SPIRV", ShaderFormat::SPIRV);
  shaderFormat.value("WGSL", ShaderFormat::WGSL);
  shaderFormat.value("DXBC", ShaderFormat::DXBC);
  shaderFormat.value("DXIL", ShaderFormat::DXIL);
  shaderFormat.value("SPIRV_1_3", ShaderFormat::SPIRV_1_3);
}
} // namespace sinen
