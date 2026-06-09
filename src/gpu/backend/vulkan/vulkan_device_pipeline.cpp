#ifndef EMSCRIPTEN
#include "vulkan_convert.hpp"
#include "vulkan_device.hpp"
#include "vulkan_resources.hpp"

#include <SDL3/SDL.h>

#include <algorithm>
#include <array>

namespace sinen::gpu::vulkan {
namespace {
VkDescriptorSetLayout createDescriptorSetLayout(
    VkDevice device, const std::vector<VkDescriptorSetLayoutBinding> &bindings,
    const char *label) {
  VkDescriptorSetLayoutCreateInfo ci{};
  ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  ci.bindingCount = static_cast<uint32_t>(bindings.size());
  ci.pBindings = bindings.data();

  VkDescriptorSetLayout layout = VK_NULL_HANDLE;
  if (vkCreateDescriptorSetLayout(device, &ci, nullptr, &layout) !=
      VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vkCreateDescriptorSetLayout (%s) failed", label);
    return VK_NULL_HANDLE;
  }
  return layout;
}

std::vector<VkDescriptorSetLayoutBinding>
uniformBindings(uint32_t count, VkShaderStageFlags stages) {
  std::vector<VkDescriptorSetLayoutBinding> bindings;
  bindings.reserve(count);
  for (uint32_t i = 0; i < count; ++i) {
    VkDescriptorSetLayoutBinding b{};
    b.binding = i;
    b.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    b.descriptorCount = 1;
    b.stageFlags = stages;
    bindings.push_back(b);
  }
  return bindings;
}

std::vector<VkDescriptorSetLayoutBinding>
samplerBindings(uint32_t count, VkShaderStageFlags stages) {
  std::vector<VkDescriptorSetLayoutBinding> bindings;
  bindings.reserve(count);
  for (uint32_t i = 0; i < count; ++i) {
    VkDescriptorSetLayoutBinding b{};
    b.binding = i;
    b.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    b.descriptorCount = 1;
    b.stageFlags = stages;
    bindings.push_back(b);
  }
  return bindings;
}

std::vector<VkDescriptorSetLayoutBinding>
storageBufferBindings(uint32_t count, VkShaderStageFlags stages) {
  std::vector<VkDescriptorSetLayoutBinding> bindings;
  bindings.reserve(count);
  for (uint32_t i = 0; i < count; ++i) {
    VkDescriptorSetLayoutBinding b{};
    b.binding = i;
    b.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    b.descriptorCount = 1;
    b.stageFlags = stages;
    bindings.push_back(b);
  }
  return bindings;
}

std::vector<VkDescriptorSetLayoutBinding>
accelerationStructureBindings(uint32_t count, VkShaderStageFlags stages) {
  std::vector<VkDescriptorSetLayoutBinding> bindings;
  bindings.reserve(count);
  for (uint32_t i = 0; i < count; ++i) {
    VkDescriptorSetLayoutBinding b{};
    b.binding = i;
    b.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    b.descriptorCount = 1;
    b.stageFlags = stages;
    bindings.push_back(b);
  }
  return bindings;
}

VkRenderPass
createCompatibleRenderPass(VkDevice device,
                           const GraphicsPipeline::CreateInfo &createInfo) {
  std::vector<VkAttachmentDescription> attachments;
  std::vector<VkAttachmentReference> colorRefs;
  attachments.reserve(createInfo.targetInfo.colorTargetDescriptions.size() + 1);
  colorRefs.reserve(createInfo.targetInfo.colorTargetDescriptions.size());

  for (const auto &target : createInfo.targetInfo.colorTargetDescriptions) {
    VkAttachmentDescription attachment{};
    attachment.format = convert::textureFormatFrom(target.format);
    attachment.samples =
        convert::sampleCountFrom(createInfo.multiSampleState.sampleCount);
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachments.push_back(attachment);

    VkAttachmentReference ref{};
    ref.attachment = static_cast<uint32_t>(attachments.size() - 1);
    ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorRefs.push_back(ref);
  }

  VkAttachmentReference depthRef{};
  VkAttachmentReference *depthRefPtr = nullptr;
  if (createInfo.targetInfo.hasDepthStencilTarget) {
    VkAttachmentDescription attachment{};
    attachment.format = convert::textureFormatFrom(
        createInfo.targetInfo.depthStencilTargetFormat);
    attachment.samples =
        convert::sampleCountFrom(createInfo.multiSampleState.sampleCount);
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachments.push_back(attachment);

    depthRef.attachment = static_cast<uint32_t>(attachments.size() - 1);
    depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthRefPtr = &depthRef;
  }

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = static_cast<uint32_t>(colorRefs.size());
  subpass.pColorAttachments = colorRefs.data();
  subpass.pDepthStencilAttachment = depthRefPtr;

  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                            VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  dependency.dstStageMask = dependency.srcStageMask;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                             VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  VkRenderPassCreateInfo ci{};
  ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  ci.attachmentCount = static_cast<uint32_t>(attachments.size());
  ci.pAttachments = attachments.data();
  ci.subpassCount = 1;
  ci.pSubpasses = &subpass;
  ci.dependencyCount = 1;
  ci.pDependencies = &dependency;

  VkRenderPass renderPass = VK_NULL_HANDLE;
  if (vkCreateRenderPass(device, &ci, nullptr, &renderPass) != VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vkCreateRenderPass (pipeline) failed");
    return VK_NULL_HANDLE;
  }
  return renderPass;
}
} // namespace

Ptr<gpu::GraphicsPipeline>
Device::createGraphicsPipeline(const GraphicsPipeline::CreateInfo &createInfo) {
  auto vs = downCast<Shader>(createInfo.vertexShader);
  auto fs = downCast<Shader>(createInfo.fragmentShader);
  const uint32_t vertexUniformCount = vs ? vs->getNumUniformBuffers() : 0u;
  const uint32_t fragmentUniformCount = fs ? fs->getNumUniformBuffers() : 0u;
  const uint32_t fragmentSamplerCount = fs ? fs->getNumSamplers() : 0u;

  VkDescriptorSetLayout vertexSamplerSetLayout =
      createDescriptorSetLayout(device, {}, "vertex sampler");
  VkDescriptorSetLayout vertexUniformSetLayout = createDescriptorSetLayout(
      device, uniformBindings(vertexUniformCount, VK_SHADER_STAGE_VERTEX_BIT),
      "vertex uniform");
  VkDescriptorSetLayout fragmentSamplerSetLayout = createDescriptorSetLayout(
      device,
      samplerBindings(fragmentSamplerCount, VK_SHADER_STAGE_FRAGMENT_BIT),
      "fragment sampler");
  VkDescriptorSetLayout fragmentUniformSetLayout = createDescriptorSetLayout(
      device,
      uniformBindings(fragmentUniformCount, VK_SHADER_STAGE_FRAGMENT_BIT),
      "fragment uniform");
  VkDescriptorSetLayout emptySetLayout = VK_NULL_HANDLE;
  VkDescriptorSetLayout accelerationStructureSetLayout = VK_NULL_HANDLE;
  if (rayQuerySupported) {
    emptySetLayout = createDescriptorSetLayout(device, {}, "ray query empty");
    accelerationStructureSetLayout = createDescriptorSetLayout(
        device, accelerationStructureBindings(8, VK_SHADER_STAGE_ALL_GRAPHICS),
        "ray query acceleration structures");
  }
  if (!vertexSamplerSetLayout || !vertexUniformSetLayout ||
      !fragmentSamplerSetLayout || !fragmentUniformSetLayout ||
      (rayQuerySupported &&
       (!emptySetLayout || !accelerationStructureSetLayout))) {
    if (accelerationStructureSetLayout)
      vkDestroyDescriptorSetLayout(device, accelerationStructureSetLayout,
                                   nullptr);
    if (emptySetLayout)
      vkDestroyDescriptorSetLayout(device, emptySetLayout, nullptr);
    if (fragmentUniformSetLayout)
      vkDestroyDescriptorSetLayout(device, fragmentUniformSetLayout, nullptr);
    if (fragmentSamplerSetLayout)
      vkDestroyDescriptorSetLayout(device, fragmentSamplerSetLayout, nullptr);
    if (vertexUniformSetLayout)
      vkDestroyDescriptorSetLayout(device, vertexUniformSetLayout, nullptr);
    if (vertexSamplerSetLayout)
      vkDestroyDescriptorSetLayout(device, vertexSamplerSetLayout, nullptr);
    return nullptr;
  }

  VkPipelineLayoutCreateInfo layoutCI{};
  layoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  std::array<VkDescriptorSetLayout, 7> rayQuerySetLayouts = {
      vertexSamplerSetLayout,
      vertexUniformSetLayout,
      fragmentSamplerSetLayout,
      fragmentUniformSetLayout,
      emptySetLayout,
      emptySetLayout,
      accelerationStructureSetLayout};
  std::array<VkDescriptorSetLayout, 4> setLayouts = {
      vertexSamplerSetLayout, vertexUniformSetLayout, fragmentSamplerSetLayout,
      fragmentUniformSetLayout};
  layoutCI.setLayoutCount = static_cast<uint32_t>(
      rayQuerySupported ? rayQuerySetLayouts.size() : setLayouts.size());
  layoutCI.pSetLayouts =
      rayQuerySupported ? rayQuerySetLayouts.data() : setLayouts.data();
  VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
  if (vkCreatePipelineLayout(device, &layoutCI, nullptr, &pipelineLayout) !=
      VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vkCreatePipelineLayout failed");
    if (accelerationStructureSetLayout)
      vkDestroyDescriptorSetLayout(device, accelerationStructureSetLayout,
                                   nullptr);
    if (emptySetLayout)
      vkDestroyDescriptorSetLayout(device, emptySetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, fragmentUniformSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, fragmentSamplerSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, vertexUniformSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, vertexSamplerSetLayout, nullptr);
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

  VkRenderPass renderPass = createCompatibleRenderPass(device, createInfo);
  if (renderPass == VK_NULL_HANDLE) {
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    if (accelerationStructureSetLayout)
      vkDestroyDescriptorSetLayout(device, accelerationStructureSetLayout,
                                   nullptr);
    if (emptySetLayout)
      vkDestroyDescriptorSetLayout(device, emptySetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, fragmentUniformSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, fragmentSamplerSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, vertexUniformSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, vertexSamplerSetLayout, nullptr);
    return nullptr;
  }

  VkGraphicsPipelineCreateInfo pipelineCI{};
  pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
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
  pipelineCI.renderPass = renderPass;
  pipelineCI.subpass = 0;

  VkPipeline pipeline = VK_NULL_HANDLE;
  if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr,
                                &pipeline) != VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vkCreateGraphicsPipelines failed");
    vkDestroyRenderPass(device, renderPass, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    if (accelerationStructureSetLayout)
      vkDestroyDescriptorSetLayout(device, accelerationStructureSetLayout,
                                   nullptr);
    if (emptySetLayout)
      vkDestroyDescriptorSetLayout(device, emptySetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, fragmentUniformSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, fragmentSamplerSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, vertexUniformSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, vertexSamplerSetLayout, nullptr);
    return nullptr;
  }

  GraphicsPipeline::LayoutInfo layoutInfo{};
  layoutInfo.vertexSamplerSetLayout = vertexSamplerSetLayout;
  layoutInfo.vertexUniformSetLayout = vertexUniformSetLayout;
  layoutInfo.fragmentSamplerSetLayout = fragmentSamplerSetLayout;
  layoutInfo.fragmentUniformSetLayout = fragmentUniformSetLayout;
  layoutInfo.emptySetLayout = emptySetLayout;
  layoutInfo.accelerationStructureSetLayout = accelerationStructureSetLayout;
  layoutInfo.pipelineLayout = pipelineLayout;
  layoutInfo.vertexUniformBindingCount = vertexUniformCount;
  layoutInfo.fragmentUniformBindingCount = fragmentUniformCount;
  layoutInfo.fragmentSamplerBindingCount = fragmentSamplerCount;

  return makePtr<GraphicsPipeline>(createInfo.allocator, createInfo, *this,
                                   pipeline, renderPass, layoutInfo);
}

Ptr<gpu::ComputePipeline>
Device::createComputePipeline(const ComputePipeline::CreateInfo &createInfo) {
  auto cs = downCast<Shader>(createInfo.computeShader);
  if (!cs) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: createComputePipeline missing shader");
    return nullptr;
  }

  const uint32_t storageBufferCount = cs->getNumStorageBuffers();
  const uint32_t uniformCount = cs->getNumUniformBuffers();

  VkDescriptorSetLayout storageSetLayout = createDescriptorSetLayout(
      device,
      storageBufferBindings(storageBufferCount, VK_SHADER_STAGE_COMPUTE_BIT),
      "compute storage buffers");
  VkDescriptorSetLayout uniformSetLayout = createDescriptorSetLayout(
      device, uniformBindings(uniformCount, VK_SHADER_STAGE_COMPUTE_BIT),
      "compute uniforms");
  VkDescriptorSetLayout emptySetLayout =
      createDescriptorSetLayout(device, {}, "compute empty");
  VkDescriptorSetLayout accelerationStructureSetLayout = VK_NULL_HANDLE;
  if (rayQuerySupported) {
    accelerationStructureSetLayout = createDescriptorSetLayout(
        device, accelerationStructureBindings(8, VK_SHADER_STAGE_COMPUTE_BIT),
        "compute ray query acceleration structures");
  }
  if (!emptySetLayout || !storageSetLayout || !uniformSetLayout ||
      (rayQuerySupported &&
       accelerationStructureSetLayout == VK_NULL_HANDLE)) {
    if (accelerationStructureSetLayout)
      vkDestroyDescriptorSetLayout(device, accelerationStructureSetLayout,
                                   nullptr);
    if (emptySetLayout)
      vkDestroyDescriptorSetLayout(device, emptySetLayout, nullptr);
    if (uniformSetLayout)
      vkDestroyDescriptorSetLayout(device, uniformSetLayout, nullptr);
    if (storageSetLayout)
      vkDestroyDescriptorSetLayout(device, storageSetLayout, nullptr);
    return nullptr;
  }

  std::array<VkDescriptorSetLayout, 7> rayQuerySetLayouts = {
      emptySetLayout,
      storageSetLayout,
      uniformSetLayout,
      emptySetLayout,
      emptySetLayout,
      emptySetLayout,
      accelerationStructureSetLayout};
  std::array<VkDescriptorSetLayout, 3> setLayouts = {
      emptySetLayout, storageSetLayout, uniformSetLayout};
  VkPipelineLayoutCreateInfo layoutCI{};
  layoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  layoutCI.setLayoutCount = static_cast<uint32_t>(
      rayQuerySupported ? rayQuerySetLayouts.size() : setLayouts.size());
  layoutCI.pSetLayouts =
      rayQuerySupported ? rayQuerySetLayouts.data() : setLayouts.data();

  VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
  if (vkCreatePipelineLayout(device, &layoutCI, nullptr, &pipelineLayout) !=
      VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vkCreatePipelineLayout (compute) failed");
    if (accelerationStructureSetLayout)
      vkDestroyDescriptorSetLayout(device, accelerationStructureSetLayout,
                                   nullptr);
    if (emptySetLayout)
      vkDestroyDescriptorSetLayout(device, emptySetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, uniformSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, storageSetLayout, nullptr);
    return nullptr;
  }

  VkPipelineShaderStageCreateInfo stage{};
  stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
  stage.module = cs->getNative();
  stage.pName = cs->getEntrypoint();

  VkComputePipelineCreateInfo pipelineCI{};
  pipelineCI.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
  pipelineCI.stage = stage;
  pipelineCI.layout = pipelineLayout;

  VkPipeline pipeline = VK_NULL_HANDLE;
  if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr,
                               &pipeline) != VK_SUCCESS) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Vulkan: vkCreateComputePipelines failed");
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    if (accelerationStructureSetLayout)
      vkDestroyDescriptorSetLayout(device, accelerationStructureSetLayout,
                                   nullptr);
    if (emptySetLayout)
      vkDestroyDescriptorSetLayout(device, emptySetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, uniformSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, storageSetLayout, nullptr);
    return nullptr;
  }

  ComputePipeline::LayoutInfo layoutInfo{};
  layoutInfo.emptySetLayout = emptySetLayout;
  layoutInfo.storageBufferSetLayout = storageSetLayout;
  layoutInfo.uniformSetLayout = uniformSetLayout;
  layoutInfo.accelerationStructureSetLayout = accelerationStructureSetLayout;
  layoutInfo.pipelineLayout = pipelineLayout;
  layoutInfo.storageBufferBindingCount = storageBufferCount;
  layoutInfo.uniformBindingCount = uniformCount;
  return makePtr<ComputePipeline>(createInfo.allocator, createInfo, *this,
                                  pipeline, layoutInfo);
}
} // namespace sinen::gpu::vulkan

#endif // EMSCRIPTEN
