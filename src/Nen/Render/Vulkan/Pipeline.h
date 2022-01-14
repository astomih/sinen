#pragma once
#include <Nen.hpp>
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
#include "PipelineLayout.h"
#include <SDL_vulkan.h>
#include <vector>
#include <vulkan/vk_layer.h>

namespace nen::vk {
class Pipeline {
public:
  Pipeline() = default;
  void Initialize(PipelineLayout &layout, VkRenderPass renderPass,
                  std::vector<VkPipelineShaderStageCreateInfo> &shaderStages);
  void Prepare(VkDevice device);
  void Bind(VkCommandBuffer command);
  void Cleanup(VkDevice device);

  void SetDepthTest(VkBool32 isEnable);
  void SetDepthWrite(VkBool32 isEnable);
  void ColorBlendFactor(VkBlendFactor src, VkBlendFactor dst);
  void AlphaBlendFactor(VkBlendFactor src, VkBlendFactor dst);

private:
  VkPipeline pipeline;
  VkPipelineColorBlendStateCreateInfo cbCI;
  VkPipelineColorBlendAttachmentState blendAttachment;
  VkPipelineDepthStencilStateCreateInfo depthStencilCI;
  VkGraphicsPipelineCreateInfo graphicsCI;
};
} // namespace nen::vk
#endif