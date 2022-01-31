#pragma once
#include <Nen.hpp>
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
#include <SDL_vulkan.h>
#include <array>
#include <vulkan/vk_layer.h>

namespace nen::vk {
class PipelineLayout {
public:
  PipelineLayout() = default;
  void Initialize(VkDevice device,
                  const VkDescriptorSetLayout *descriptorLayout,
                  const VkExtent2D &extent);
  void Prepare(VkDevice device);
  void Cleanup(VkDevice device);
  const VkPipelineInputAssemblyStateCreateInfo *GetInputAssemblyCI() {
    return &inputAssemblyCI;
  }
  const VkPipelineViewportStateCreateInfo *GetViewportCI() {
    return &viewportCI;
  };
  const VkPipelineRasterizationStateCreateInfo *GetRasterizerCI() {
    return &rasterizerCI;
  };
  const VkPipelineMultisampleStateCreateInfo *GetMultisampleCI() {
    return &multisampleCI;
  };
  const VkPipelineDepthStencilStateCreateInfo *GetDepthStencilCI() {
    return &depthStencilCI;
  };
  const VkPipelineLayoutCreateInfo *GetPipelineLayoutCI() {
    return &pipelineLayoutCI;
  };
  const VkPipelineVertexInputStateCreateInfo *GetVertexInputCI() {
    return &vertexInputCI;
  };
  const VkPipelineDynamicStateCreateInfo *GetDynamicStateCI() {
    return &pipelineDynamicStateCI;
  };
  VkPipelineLayout GetLayout() { return layout; }

  void change_viewport(VkRect2D _scissor, VkViewport _viewport) {
    viewport = _viewport;
    scissor = _scissor;
  }

private:
  VkPipelineLayout layout;
  VkVertexInputBindingDescription inputBinding;
  std::array<VkVertexInputBindingDescription, 2> vibDisc;
  std::array<VkVertexInputAttributeDescription, 4> inputAttribs;
  VkPipelineVertexInputStateCreateInfo vertexInputCI;
  int colorMask;
  VkPipelineColorBlendAttachmentState blendAttachment;
  VkPipelineColorBlendStateCreateInfo cbCI;
  VkViewport viewport;
  VkRect2D scissor;
  VkPipelineViewportStateCreateInfo viewportCI;
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyCI;
  VkPipelineRasterizationStateCreateInfo rasterizerCI;
  VkPipelineMultisampleStateCreateInfo multisampleCI;
  VkPipelineDepthStencilStateCreateInfo depthStencilCI;
  VkPipelineLayoutCreateInfo pipelineLayoutCI;
  VkPipelineDynamicStateCreateInfo pipelineDynamicStateCI;
};
} // namespace nen::vk
#endif