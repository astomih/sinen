#include "IO/AssetReader.hpp"
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
#include "VKBase.h"
#include "VulkanShader.h"
#include <fstream>

namespace nen::vk {
VkPipelineShaderStageCreateInfo
VulkanShader::LoadModule(VkDevice device, const char *fileName,
                         VkShaderStageFlagBits stage) {
  std::string filedata =
      asset_reader::LoadAsString(asset_type::vk_shader, fileName);

  VkShaderModule shaderModule;
  VkShaderModuleCreateInfo ci{};
  ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  ci.pCode = reinterpret_cast<uint32_t *>(filedata.data());
  ci.codeSize = filedata.size();
  vkCreateShaderModule(device, &ci, nullptr, &shaderModule);
  VkPipelineShaderStageCreateInfo shaderStageCI{};
  shaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStageCI.stage = stage;
  shaderStageCI.module = shaderModule;
  shaderStageCI.pName = "main";
  return shaderStageCI;
}

void VulkanShader::CleanModule(
    VkDevice device,
    std::vector<VkPipelineShaderStageCreateInfo> &shaderStages) {
  for (const auto &v : shaderStages) {
    vkDestroyShaderModule(device, v.module, nullptr);
  }
}
} // namespace nen::vk
#endif