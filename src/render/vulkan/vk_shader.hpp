#pragma once
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
#include <vector>
#include <vulkan/vk_layer.h>

namespace nen {
class vk_shader {
public:
  vk_shader() = default;
  static VkPipelineShaderStageCreateInfo
  load(VkDevice device, const char *fileName, VkShaderStageFlagBits stage);
  static void clean(VkDevice device,
                    std::vector<VkPipelineShaderStageCreateInfo> &shaderStages);

private:
};
} // namespace nen
#endif