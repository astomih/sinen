#ifndef EMSCRIPTEN
#include "vulkan_convert.hpp"
#include "vulkan_device.hpp"
#include "vulkan_resources.hpp"

#include <SDL3/SDL.h>

#include <algorithm>
#include <array>

namespace sinen::gpu::vulkan {
Ptr<gpu::GraphicsPipeline>
Device::createGraphicsPipeline(const GraphicsPipeline::CreateInfo &createInfo) {
  auto vs = downCast<Shader>(createInfo.vertexShader);
  auto fs = downCast<Shader>(createInfo.fragmentShader);
  uint32_t uniformCount = std::max(vs ? vs->getNumUniformBuffers() : 0u,
                                   fs ? fs->getNumUniformBuffers() : 0u);
  uint32_t samplerCount = fs ? fs->getNumSamplers() : 0u;

  // set = 0: unused/empty
  VkDescriptorSetLayoutCreateInfo emptyLayoutCI{};
  emptyLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  emptyLayoutCI.bindingCount = 0;
  emptyLayoutCI.pBindings = nullptr;
  VkDescriptorSetLayout emptySetLayout = VK_NULL_HANDLE;
  if (vkCreateDescriptorSetLayout(device, &emptyLayoutCI, nullptr,
                                  &emptySetLayout) != VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vkCreateDescriptorSetLayout (empty) failed");
    return nullptr;
  }

  // set = 1: uniform buffers
  std::vector<VkDescriptorSetLayoutBinding> uniformBindings;
  uniformBindings.reserve(uniformCount);
  for (uint32_t i = 0; i < uniformCount; ++i) {
    VkDescriptorSetLayoutBinding b{};
    b.binding = i;
    b.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    b.descriptorCount = 1;
    b.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    uniformBindings.push_back(b);
  }

  VkDescriptorSetLayoutCreateInfo uniformLayoutCI{};
  uniformLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  uniformLayoutCI.bindingCount = static_cast<uint32_t>(uniformBindings.size());
  uniformLayoutCI.pBindings = uniformBindings.data();
  VkDescriptorSetLayout uniformSetLayout = VK_NULL_HANDLE;
  if (vkCreateDescriptorSetLayout(device, &uniformLayoutCI, nullptr,
                                  &uniformSetLayout) != VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vkCreateDescriptorSetLayout (uniform) failed");
    vkDestroyDescriptorSetLayout(device, emptySetLayout, nullptr);
    return nullptr;
  }

  // set = 2: samplers
  std::vector<VkDescriptorSetLayoutBinding> samplerBindings;
  samplerBindings.reserve(samplerCount);
  for (uint32_t i = 0; i < samplerCount; ++i) {
    VkDescriptorSetLayoutBinding b{};
    b.binding = i;
    b.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    b.descriptorCount = 1;
    b.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerBindings.push_back(b);
  }
  VkDescriptorSetLayoutCreateInfo samplerLayoutCI{};
  samplerLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  samplerLayoutCI.bindingCount = static_cast<uint32_t>(samplerBindings.size());
  samplerLayoutCI.pBindings = samplerBindings.data();
  VkDescriptorSetLayout samplerSetLayout = VK_NULL_HANDLE;
  if (vkCreateDescriptorSetLayout(device, &samplerLayoutCI, nullptr,
                                  &samplerSetLayout) != VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vkCreateDescriptorSetLayout (sampler) failed");
    vkDestroyDescriptorSetLayout(device, uniformSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, emptySetLayout, nullptr);
    return nullptr;
  }

  VkPipelineLayoutCreateInfo layoutCI{};
  layoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  std::array<VkDescriptorSetLayout, 3> setLayouts = {
      emptySetLayout, uniformSetLayout, samplerSetLayout};
  layoutCI.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
  layoutCI.pSetLayouts = setLayouts.data();
  VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
  if (vkCreatePipelineLayout(device, &layoutCI, nullptr, &pipelineLayout) !=
      VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vkCreatePipelineLayout failed");
    vkDestroyDescriptorSetLayout(device, samplerSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, uniformSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, emptySetLayout, nullptr);
    return nullptr;
  }

  VkPipelineShaderStageCreateInfo stages[2]{};
  stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  stages[0].module = vs->getNative();
  stages[0].pName = vs->getEntrypoint();

  stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  stages[1].module = fs->getNative();
  stages[1].pName = fs->getEntrypoint();

  std::vector<VkVertexInputBindingDescription> bindingDescs;
  bindingDescs.reserve(
      createInfo.vertexInputState.vertexBufferDescriptions.size());
  for (auto &desc : createInfo.vertexInputState.vertexBufferDescriptions) {
    VkVertexInputBindingDescription b{};
    b.binding = desc.slot;
    b.stride = desc.pitch;
    b.inputRate = convert::vertexInputRateFrom(desc.inputRate);
    bindingDescs.push_back(b);
  }

  std::vector<VkVertexInputAttributeDescription> attrDescs;
  attrDescs.reserve(createInfo.vertexInputState.vertexAttributes.size());
  for (auto &attr : createInfo.vertexInputState.vertexAttributes) {
    VkVertexInputAttributeDescription a{};
    a.location = attr.location;
    a.binding = attr.bufferSlot;
    a.format = convert::vertexElementFormatFrom(attr.format);
    a.offset = attr.offset;
    attrDescs.push_back(a);
  }

  VkPipelineVertexInputStateCreateInfo vertexInput{};
  vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInput.vertexBindingDescriptionCount =
      static_cast<uint32_t>(bindingDescs.size());
  vertexInput.pVertexBindingDescriptions = bindingDescs.data();
  vertexInput.vertexAttributeDescriptionCount =
      static_cast<uint32_t>(attrDescs.size());
  vertexInput.pVertexAttributeDescriptions = attrDescs.data();

  VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
  inputAssembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology =
      convert::primitiveTopologyFrom(createInfo.primitiveType);
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  VkPipelineViewportStateCreateInfo viewportState{};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.scissorCount = 1;

  VkPipelineRasterizationStateCreateInfo raster{};
  raster.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  raster.depthClampEnable =
      createInfo.rasterizerState.enableDepthClip ? VK_TRUE : VK_FALSE;
  raster.rasterizerDiscardEnable = VK_FALSE;
  raster.polygonMode =
      convert::polygonModeFrom(createInfo.rasterizerState.fillMode);
  raster.cullMode = convert::cullModeFrom(createInfo.rasterizerState.cullMode);
  raster.frontFace =
      convert::frontFaceFrom(createInfo.rasterizerState.frontFace);
  raster.depthBiasEnable =
      createInfo.rasterizerState.enableDepthBias ? VK_TRUE : VK_FALSE;
  raster.depthBiasConstantFactor =
      createInfo.rasterizerState.depthBiasConstantFactor;
  raster.depthBiasClamp = createInfo.rasterizerState.depthBiasClamp;
  raster.depthBiasSlopeFactor = createInfo.rasterizerState.depthBiasSlopeFactor;
  raster.lineWidth = 1.0f;

  VkPipelineMultisampleStateCreateInfo msaa{};
  msaa.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  msaa.rasterizationSamples =
      convert::sampleCountFrom(createInfo.multiSampleState.sampleCount);

  VkPipelineDepthStencilStateCreateInfo depthStencil{};
  depthStencil.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencil.depthTestEnable =
      createInfo.depthStencilState.enableDepthTest ? VK_TRUE : VK_FALSE;
  depthStencil.depthWriteEnable =
      createInfo.depthStencilState.enableDepthWrite ? VK_TRUE : VK_FALSE;
  depthStencil.depthCompareOp =
      convert::compareOpFrom(createInfo.depthStencilState.compareOp);
  depthStencil.stencilTestEnable =
      createInfo.depthStencilState.enableStencilTest ? VK_TRUE : VK_FALSE;

  std::vector<VkPipelineColorBlendAttachmentState> blendAttachments;
  blendAttachments.reserve(
      createInfo.targetInfo.colorTargetDescriptions.size());
  for (auto &target : createInfo.targetInfo.colorTargetDescriptions) {
    VkPipelineColorBlendAttachmentState blend{};
    blend.colorWriteMask = target.blendState.colorWriteMask;
    blend.blendEnable = target.blendState.enableBlend ? VK_TRUE : VK_FALSE;
    blend.srcColorBlendFactor =
        convert::blendFactorFrom(target.blendState.srcColorBlendFactor);
    blend.dstColorBlendFactor =
        convert::blendFactorFrom(target.blendState.dstColorBlendFactor);
    blend.colorBlendOp = convert::blendOpFrom(target.blendState.colorBlendOp);
    blend.srcAlphaBlendFactor =
        convert::blendFactorFrom(target.blendState.srcAlphaBlendFactor);
    blend.dstAlphaBlendFactor =
        convert::blendFactorFrom(target.blendState.dstAlphaBlendFactor);
    blend.alphaBlendOp = convert::blendOpFrom(target.blendState.alphaBlendOp);
    blendAttachments.push_back(blend);
  }

  VkPipelineColorBlendStateCreateInfo colorBlend{};
  colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlend.attachmentCount = static_cast<uint32_t>(blendAttachments.size());
  colorBlend.pAttachments = blendAttachments.data();

  std::array<VkDynamicState, 2> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT,
                                                 VK_DYNAMIC_STATE_SCISSOR};
  VkPipelineDynamicStateCreateInfo dynamic{};
  dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
  dynamic.pDynamicStates = dynamicStates.data();

  std::vector<VkFormat> colorFormats;
  colorFormats.reserve(createInfo.targetInfo.colorTargetDescriptions.size());
  for (auto &target : createInfo.targetInfo.colorTargetDescriptions) {
    colorFormats.push_back(convert::textureFormatFrom(target.format));
  }
  VkPipelineRenderingCreateInfo renderingInfo{};
  renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
  renderingInfo.colorAttachmentCount =
      static_cast<uint32_t>(colorFormats.size());
  renderingInfo.pColorAttachmentFormats = colorFormats.data();
  renderingInfo.depthAttachmentFormat =
      createInfo.targetInfo.hasDepthStencilTarget
          ? convert::textureFormatFrom(
                createInfo.targetInfo.depthStencilTargetFormat)
          : VK_FORMAT_UNDEFINED;
  renderingInfo.stencilAttachmentFormat = renderingInfo.depthAttachmentFormat;

  VkGraphicsPipelineCreateInfo pipelineCI{};
  pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineCI.pNext = &renderingInfo;
  pipelineCI.stageCount = 2;
  pipelineCI.pStages = stages;
  pipelineCI.pVertexInputState = &vertexInput;
  pipelineCI.pInputAssemblyState = &inputAssembly;
  pipelineCI.pViewportState = &viewportState;
  pipelineCI.pRasterizationState = &raster;
  pipelineCI.pMultisampleState = &msaa;
  pipelineCI.pDepthStencilState =
      createInfo.targetInfo.hasDepthStencilTarget ? &depthStencil : nullptr;
  pipelineCI.pColorBlendState = &colorBlend;
  pipelineCI.pDynamicState = &dynamic;
  pipelineCI.layout = pipelineLayout;

  VkPipeline pipeline = VK_NULL_HANDLE;
  if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr,
                                &pipeline) != VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vkCreateGraphicsPipelines failed");
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, samplerSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, uniformSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, emptySetLayout, nullptr);
    return nullptr;
  }

  GraphicsPipeline::LayoutInfo layoutInfo{};
  layoutInfo.emptySetLayout = emptySetLayout;
  layoutInfo.uniformSetLayout = uniformSetLayout;
  layoutInfo.samplerSetLayout = samplerSetLayout;
  layoutInfo.pipelineLayout = pipelineLayout;
  layoutInfo.uniformBindingCount = uniformCount;
  layoutInfo.samplerBindingCount = samplerCount;

  return makePtr<GraphicsPipeline>(createInfo.allocator, createInfo, *this,
                                   pipeline, layoutInfo);
}

Ptr<gpu::ComputePipeline> Device::createComputePipeline(
    const ComputePipeline::CreateInfo & /*createInfo*/) {
  return nullptr;
}
} // namespace sinen::gpu::vulkan

#endif // EMSCRIPTEN
