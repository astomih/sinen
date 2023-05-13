#include "vk_shader.hpp"
#include <fstream>
#include <io/data_stream.hpp>

namespace sinen {
VkPipelineShaderStageCreateInfo vk_shader::load(VkDevice device,
                                                const char *fileName,
                                                VkShaderStageFlagBits stage) {
  std::string filedata =
      data_stream::open_as_string(asset_type::vk_shader, fileName);

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

void vk_shader::clean(
    VkDevice device,
    std::vector<VkPipelineShaderStageCreateInfo> &shaderStages) {
  for (const auto &v : shaderStages) {
    vkDestroyShaderModule(device, v.module, nullptr);
  }
}
} // namespace sinen
