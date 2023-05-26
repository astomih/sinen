#ifndef SINEN_VK_SHADER_HPP
#define SINEN_VK_SHADER_HPP
#include <vector>
#include <vulkan/vk_layer.h>

namespace sinen {
class vk_shader {
public:
  vk_shader() = default;
  static VkPipelineShaderStageCreateInfo
  load(VkDevice device, const char *fileName, VkShaderStageFlagBits stage);
  static void clean(VkDevice device,
                    std::vector<VkPipelineShaderStageCreateInfo> &shaderStages);

private:
};
} // namespace sinen
#endif // SINEN_VK_SHADER_HPP
