#ifndef SINEN_VK_PIPELINE_LAYOUT
#define SINEN_VK_PIPELINE_LAYOUT
#include <SDL3/SDL_vulkan.h>
#include <array>
#include <vector>
#include <vulkan/vk_layer.h>

namespace sinen {
class vk_pipeline_layout {
public:
  vk_pipeline_layout() = default;
  void initialize(VkDevice device,
                  const VkDescriptorSetLayout *descriptorLayout,
                  const VkExtent2D &extent, bool instance = true);
  void prepare(VkDevice device);
  void cleanup(VkDevice device);
  const VkPipelineInputAssemblyStateCreateInfo *GetInputAssemblyCI() {
    return &inputAssemblyCI;
  }
  const VkPipelineViewportStateCreateInfo *GetViewportCI() {
    return &viewportCI;
  };
  const VkPipelineRasterizationStateCreateInfo *GetRasterizerCI() {
    return &rasterizerCI;
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
  std::vector<VkVertexInputBindingDescription> vibDesc;
  std::vector<VkVertexInputAttributeDescription> inputAttribs;
  VkPipelineVertexInputStateCreateInfo vertexInputCI;
  int colorMask;
  VkPipelineColorBlendAttachmentState blendAttachment;
  VkPipelineColorBlendStateCreateInfo cbCI;
  VkViewport viewport;
  VkRect2D scissor;
  VkPipelineViewportStateCreateInfo viewportCI;
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyCI;
  VkPipelineRasterizationStateCreateInfo rasterizerCI;
  VkPipelineDepthStencilStateCreateInfo depthStencilCI;
  VkPipelineLayoutCreateInfo pipelineLayoutCI;
  VkPipelineDynamicStateCreateInfo pipelineDynamicStateCI;
  std::vector<VkDynamicState> dynamicStates;
};
} // namespace sinen
#endif // !SINEN_VK_PIPELINE_LAYOUT