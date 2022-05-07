#if !defined(EMSCRIPTEN) && !defined(MOBILE)
#include "vk_pipeline.hpp"
#include "vk_base.hpp"
#include "vk_util.hpp"

namespace nen {
void vk_pipeline::Initialize(
    vk_pipeline_layout &layout, VkRenderPass renderPass,
    std::vector<VkPipelineShaderStageCreateInfo> &shaderStages) {
  // Blending settings
  const auto colorWriteAll =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  blendAttachment = VkPipelineColorBlendAttachmentState{
      .blendEnable = VK_TRUE,
      .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
      .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
      .colorBlendOp = VK_BLEND_OP_ADD,
      .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
      .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
      .alphaBlendOp = VK_BLEND_OP_ADD,
      .colorWriteMask = colorWriteAll};
  cbCI = VkPipelineColorBlendStateCreateInfo{};
  cbCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  cbCI.attachmentCount = 1;
  cbCI.pAttachments = &blendAttachment;

  // Depth stencil state settings
  depthStencilCI = VkPipelineDepthStencilStateCreateInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      .depthTestEnable = VK_TRUE,
      .depthWriteEnable = VK_TRUE,
      .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
      .stencilTestEnable = VK_FALSE};

  // Create pipeline
  graphicsCI = VkGraphicsPipelineCreateInfo{
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .stageCount = uint32_t(shaderStages.size()),
      .pStages = shaderStages.data(),
      .pVertexInputState = layout.GetVertexInputCI(),
      .pInputAssemblyState = layout.GetInputAssemblyCI(),
      .pViewportState = layout.GetViewportCI(),
      .pRasterizationState = layout.GetRasterizerCI(),
      .pMultisampleState = layout.GetMultisampleCI(),
      .pDepthStencilState = &depthStencilCI,
      .pColorBlendState = &cbCI,
      .pDynamicState = layout.GetDynamicStateCI(),
      .layout = layout.GetLayout(),
      .renderPass = renderPass,
  };
}
void vk_pipeline::Prepare(VkDevice device) {
  VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1,
                                              &graphicsCI, nullptr, &pipeline);
}

void vk_pipeline::Bind(VkCommandBuffer command) {
  vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

void vk_pipeline::Cleanup(VkDevice device) {
  nen::vkutil::DestroyVulkanObject<VkPipeline>(device, pipeline,
                                               &vkDestroyPipeline);
}

void vk_pipeline::SetDepthTest(VkBool32 isEnable) {
  depthStencilCI.depthTestEnable = isEnable;
}
void vk_pipeline::SetDepthWrite(VkBool32 isEnable) {
  depthStencilCI.depthWriteEnable = isEnable;
}

void vk_pipeline::ColorBlendFactor(VkBlendFactor src, VkBlendFactor dst) {
  blendAttachment.srcColorBlendFactor = src;
  blendAttachment.dstColorBlendFactor = dst;
}
void vk_pipeline::AlphaBlendFactor(VkBlendFactor src, VkBlendFactor dst) {
  blendAttachment.srcAlphaBlendFactor = src;
  blendAttachment.dstAlphaBlendFactor = dst;
}
} // namespace nen
#endif