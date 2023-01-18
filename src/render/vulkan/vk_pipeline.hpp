#ifndef SINEN_VK_PIPELINE_HPP
#define SINEN_VK_PIPELINE_HPP
#if !defined(EMSCRIPTEN) && !defined(ANDROID)
#include "vk_pipeline_layout.hpp"
#include <SDL_vulkan.h>
#include <vector>
#include <vulkan/vk_layer.h>

namespace sinen {
class vk_pipeline {
public:
  vk_pipeline() = default;
  void initialize(vk_pipeline_layout &layout, VkRenderPass renderPass,
                  std::vector<VkPipelineShaderStageCreateInfo> &shaderStages);
  void prepare(VkDevice device);
  void Bind(VkCommandBuffer command);
  void Cleanup(VkDevice device);

  void set_depth_test(VkBool32 isEnable);
  void set_depth_write(VkBool32 isEnable);
  void color_blend_factor(VkBlendFactor src, VkBlendFactor dst);
  void alpha_blend_factor(VkBlendFactor src, VkBlendFactor dst);

private:
  VkPipeline pipeline;
  VkPipelineColorBlendStateCreateInfo cbCI;
  VkPipelineColorBlendAttachmentState blendAttachment;
  VkPipelineDepthStencilStateCreateInfo depthStencilCI;
  VkGraphicsPipelineCreateInfo graphicsCI;
};
} // namespace sinen
#endif
#endif // !SINEN_VK_PIPELINE_HPP
