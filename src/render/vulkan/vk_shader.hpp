#pragma once
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
#include <vector>
#include <vulkan/vk_layer.h>

namespace nen::vk {
class vk_shader {
public:
  vk_shader() = default;
  static VkPipelineShaderStageCreateInfo
  LoadModule(VkDevice device, const char *fileName,
             VkShaderStageFlagBits stage);
  static void
  CleanModule(VkDevice device,
              std::vector<VkPipelineShaderStageCreateInfo> &shaderStages);

private:
};
} // namespace nen::vk
#endif