#include "luaapi.hpp"
#include <gpu/shader/shader_stage.hpp>

namespace sinen {
void registerShaderStage(luau::table &sn) {
  auto shaderStage = sn.get_or_create_table("ShaderStage");
  shaderStage.value("Vertex", ShaderStage::Vertex);
  shaderStage.value("Fragment", ShaderStage::Fragment);
  shaderStage.value("Compute", ShaderStage::Compute);
  shaderStage.value("RayGeneration", ShaderStage::RayGeneration);
  shaderStage.value("AnyHit", ShaderStage::AnyHit);
  shaderStage.value("ClosestHit", ShaderStage::ClosestHit);
  shaderStage.value("Miss", ShaderStage::Miss);
  shaderStage.value("Intersection", ShaderStage::Intersection);
  shaderStage.value("Callable", ShaderStage::Callable);
}
} // namespace sinen
