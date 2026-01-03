#include "sdlgpu_device.hpp"

#ifndef EMSCRIPTEN
#include "sdlgpu_buffer.hpp"
#include "sdlgpu_command_buffer.hpp"
#include "sdlgpu_compute_pipeline.hpp"
#include "sdlgpu_convert.hpp"
#include "sdlgpu_graphics_pipeline.hpp"
#include "sdlgpu_sampler.hpp"
#include "sdlgpu_shader.hpp"
#include "sdlgpu_texture.hpp"
#include "sdlgpu_transfer_buffer.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

namespace sinen::rhi::sdlgpu {
Device::~Device() {
  if (window)
    SDL_ReleaseWindowFromGPUDevice(device, window);
  SDL_DestroyGPUDevice(device);
}

void Device::claimWindow(void *window) {
  if (!SDL_ClaimWindowForGPUDevice(device, static_cast<SDL_Window *>(window))) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Failed to claim window for GPU device:\n");
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, SDL_GetError());
  }
  this->window = static_cast<SDL_Window *>(window);
}

Ptr<rhi::TransferBuffer>
Device::createTransferBuffer(const TransferBuffer::CreateInfo &createInfo) {
  SDL_GPUTransferBufferCreateInfo stagingTextureBufferCI{};
  stagingTextureBufferCI.usage =
      convert::TransferBufferUsageFrom(createInfo.usage);
  stagingTextureBufferCI.size = createInfo.size;

  SDL_GPUTransferBuffer *stagingTextureBuffer =
      SDL_CreateGPUTransferBuffer(device, &stagingTextureBufferCI);
  return makePtr<TransferBuffer>(createInfo.allocator, createInfo,
                                 downCast<Device>(getPtr()),
                                 stagingTextureBuffer);
}

Ptr<rhi::Buffer> Device::createBuffer(const Buffer::CreateInfo &createInfo) {
  SDL_GPUBufferCreateInfo bufferCI{};
  bufferCI.usage = convert::BufferUsageFrom(createInfo.usage);
  bufferCI.size = createInfo.size;
  SDL_GPUBuffer *buffer = SDL_CreateGPUBuffer(device, &bufferCI);
  return makePtr<Buffer>(createInfo.allocator, createInfo,
                         downCast<Device>(getPtr()), buffer);
}

Ptr<rhi::Texture> Device::createTexture(const Texture::CreateInfo &createInfo) {
  SDL_GPUTextureCreateInfo textureCreateInfo = {
      .type = convert::TextureTypeFrom(createInfo.type),
      .format = convert::TextureFormatFrom(createInfo.format),
      .usage = convert::TextureUsageFrom(createInfo.usage),
      .width = createInfo.width,
      .height = createInfo.height,
      .layer_count_or_depth = createInfo.layerCountOrDepth,
      .num_levels = createInfo.numLevels,
      .sample_count = convert::SampleCountFrom(createInfo.sampleCount),
  };

  SDL_GPUTexture *texture = SDL_CreateGPUTexture(device, &textureCreateInfo);
  return makePtr<Texture>(createInfo.allocator, createInfo,
                          downCast<Device>(getPtr()), texture, false);
}

Ptr<rhi::Sampler> Device::createSampler(const Sampler::CreateInfo &createInfo) {
  SDL_GPUSamplerCreateInfo samplerCreateInfo = {
      .min_filter = convert::FilterFrom(createInfo.minFilter),
      .mag_filter = convert::FilterFrom(createInfo.magFilter),
      .mipmap_mode = convert::MipmapModeFrom(createInfo.mipmapMode),
      .address_mode_u = convert::AddressModeFrom(createInfo.addressModeU),
      .address_mode_v = convert::AddressModeFrom(createInfo.addressModeV),
      .address_mode_w = convert::AddressModeFrom(createInfo.addressModeW),
  };
  SDL_GPUSampler *sampler = SDL_CreateGPUSampler(device, &samplerCreateInfo);
  return makePtr<Sampler>(createInfo.allocator, createInfo,
                          downCast<Device>(getPtr()), sampler);
}

Ptr<rhi::Shader> Device::createShader(const Shader::CreateInfo &createInfo) {
  SDL_GPUShaderCreateInfo shaderCI = {};
  shaderCI.stage = createInfo.stage == ShaderStage::Vertex
                       ? SDL_GPU_SHADERSTAGE_VERTEX
                       : SDL_GPU_SHADERSTAGE_FRAGMENT;
  shaderCI.code_size = createInfo.size;
  shaderCI.code = reinterpret_cast<const Uint8 *>(createInfo.data);
  shaderCI.format = SDL_GPU_SHADERFORMAT_SPIRV;
  shaderCI.entrypoint = createInfo.entrypoint;
  shaderCI.num_samplers = createInfo.numSamplers;
  shaderCI.num_storage_buffers = createInfo.numStorageBuffers;
  shaderCI.num_storage_textures = createInfo.numStorageTextures;
  shaderCI.num_uniform_buffers = createInfo.numUniformBuffers;

  auto *shader = SDL_CreateGPUShader(device, &shaderCI);

  auto p = downCast<Device>(getPtr());

  return makePtr<Shader>(createInfo.allocator, createInfo, p, shader);
}

Ptr<rhi::CommandBuffer>
Device::acquireCommandBuffer(const CommandBuffer::CreateInfo &createInfo) {
  SDL_GPUCommandBuffer *commandBuffer = SDL_AcquireGPUCommandBuffer(device);
  return makePtr<CommandBuffer>(createInfo.allocator, createInfo,
                                commandBuffer);
}

Ptr<rhi::GraphicsPipeline>
Device::createGraphicsPipeline(const GraphicsPipeline::CreateInfo &createInfo) {
  SDL_GPUGraphicsPipelineCreateInfo pipelineCI = {};
  pipelineCI.vertex_shader =
      downCast<Shader>(createInfo.vertexShader)->getNative();
  pipelineCI.fragment_shader =
      downCast<Shader>(createInfo.fragmentShader)->getNative();
  {
    auto &rasterizerState = createInfo.rasterizerState;
    auto &rasterizer_state = pipelineCI.rasterizer_state;
    rasterizer_state.fill_mode =
        convert::FillModeFrom(createInfo.rasterizerState.fillMode);
    rasterizer_state.cull_mode =
        convert::CullModeFrom(createInfo.rasterizerState.cullMode);
    rasterizer_state.front_face =
        convert::FrontFaceFrom(createInfo.rasterizerState.frontFace);
    rasterizer_state.depth_bias_constant_factor =
        rasterizerState.depthBiasConstantFactor;
    rasterizer_state.depth_bias_clamp = rasterizerState.depthBiasClamp;
    rasterizer_state.depth_bias_slope_factor =
        rasterizerState.depthBiasSlopeFactor;
    rasterizer_state.enable_depth_bias = rasterizerState.enableDepthBias;
    rasterizer_state.enable_depth_clip = rasterizerState.enableDepthClip;
  }
  {
    auto &multisample_state = pipelineCI.multisample_state;
    auto &multiSampleState = createInfo.multiSampleState;
    multisample_state.enable_mask = multiSampleState.enableMask;
    multisample_state.sample_count =
        convert::SampleCountFrom(multiSampleState.sampleCount);
    multisample_state.sample_mask = multiSampleState.sampleMask;
  }
  {
    auto &depth_stencil_state = pipelineCI.depth_stencil_state;
    auto &depthStencilState = createInfo.depthStencilState;
    depth_stencil_state.compare_op =
        convert::CompareOpFrom(depthStencilState.compareOp);

    depth_stencil_state.back_stencil_state.fail_op =
        convert::StencilOpFrom(depthStencilState.backStencilState.failOp);
    depth_stencil_state.back_stencil_state.pass_op =
        convert::StencilOpFrom(depthStencilState.backStencilState.passOp);
    depth_stencil_state.back_stencil_state.depth_fail_op =
        convert::StencilOpFrom(depthStencilState.backStencilState.depthFailOp);
    depth_stencil_state.back_stencil_state.compare_op =
        convert::CompareOpFrom(depthStencilState.backStencilState.compareOp);

    depth_stencil_state.front_stencil_state.fail_op =
        convert::StencilOpFrom(depthStencilState.frontStencilState.failOp);
    depth_stencil_state.front_stencil_state.pass_op =
        convert::StencilOpFrom(depthStencilState.frontStencilState.passOp);
    depth_stencil_state.front_stencil_state.depth_fail_op =
        convert::StencilOpFrom(depthStencilState.frontStencilState.depthFailOp);
    depth_stencil_state.front_stencil_state.compare_op =
        convert::CompareOpFrom(depthStencilState.frontStencilState.compareOp);

    depth_stencil_state.compare_mask = depthStencilState.compareMask;
    depth_stencil_state.write_mask = depthStencilState.writeMask;
    depth_stencil_state.enable_depth_test = depthStencilState.enableDepthTest;
    depth_stencil_state.enable_depth_write = depthStencilState.enableDepthWrite;
    depth_stencil_state.enable_stencil_test =
        depthStencilState.enableStencilTest;
  }

  pipelineCI.primitive_type =
      convert::PrimitiveTypeFrom(createInfo.primitiveType);

  auto numColorTargets = createInfo.targetInfo.colorTargetDescriptions.size();
  pipelineCI.target_info.num_color_targets = numColorTargets;

  Array<SDL_GPUColorTargetDescription> colorTargetDescs(createInfo.allocator);
  colorTargetDescs.resize(numColorTargets);
  for (int i = 0; i < numColorTargets; ++i) {
    SDL_GPUColorTargetDescription colorTargetDesc{};
    colorTargetDesc.format = convert::TextureFormatFrom(
        createInfo.targetInfo.colorTargetDescriptions[i].format);
    auto &pxBlend = createInfo.targetInfo.colorTargetDescriptions[i].blendState;
    auto &blend = colorTargetDesc.blend_state;

    blend.src_alpha_blendfactor =
        convert::BlendFactorFrom(pxBlend.srcAlphaBlendFactor);
    blend.dst_alpha_blendfactor =
        convert::BlendFactorFrom(pxBlend.dstAlphaBlendFactor);
    blend.color_blend_op = convert::BlendOpFrom(pxBlend.colorBlendOp);
    blend.src_color_blendfactor =
        convert::BlendFactorFrom(pxBlend.srcColorBlendFactor);
    blend.dst_color_blendfactor =
        convert::BlendFactorFrom(pxBlend.dstColorBlendFactor);
    blend.alpha_blend_op = convert::BlendOpFrom(pxBlend.alphaBlendOp);
    blend.color_write_mask = pxBlend.colorWriteMask;
    blend.enable_blend = pxBlend.enableBlend;
    blend.enable_color_write_mask = pxBlend.enableColorWriteMask;
    colorTargetDescs[i] = colorTargetDesc;
  }
  pipelineCI.target_info.color_target_descriptions = colorTargetDescs.data();
  pipelineCI.target_info.num_color_targets = colorTargetDescs.size();
  pipelineCI.target_info.has_depth_stencil_target =
      createInfo.targetInfo.hasDepthStencilTarget;
  pipelineCI.target_info.depth_stencil_format = convert::TextureFormatFrom(
      createInfo.targetInfo.depthStencilTargetFormat);
  pipelineCI.vertex_input_state.num_vertex_attributes =
      createInfo.vertexInputState.vertexAttributes.size();
  pipelineCI.vertex_input_state.num_vertex_buffers =
      createInfo.vertexInputState.vertexBufferDescriptions.size();
  Array<SDL_GPUVertexAttribute> vertexAttributes(createInfo.allocator);
  for (int i = 0; i < createInfo.vertexInputState.vertexAttributes.size();
       ++i) {
    SDL_GPUVertexAttribute vertexAttribute = {};
    vertexAttribute.location =
        createInfo.vertexInputState.vertexAttributes[i].location;
    vertexAttribute.buffer_slot =
        createInfo.vertexInputState.vertexAttributes[i].bufferSlot;
    vertexAttribute.format = convert::VertexElementFormatFrom(
        createInfo.vertexInputState.vertexAttributes[i].format);
    vertexAttribute.offset =
        createInfo.vertexInputState.vertexAttributes[i].offset;
    vertexAttributes.push_back(vertexAttribute);
  }
  pipelineCI.vertex_input_state.vertex_attributes = vertexAttributes.data();
  Array<SDL_GPUVertexBufferDescription> vbDescs(createInfo.allocator);
  for (int i = 0;
       i < createInfo.vertexInputState.vertexBufferDescriptions.size(); i++) {
    auto &desc = createInfo.vertexInputState.vertexBufferDescriptions[i];
    SDL_GPUVertexBufferDescription vbDesc = {};
    vbDesc.input_rate = convert::VertexInputRateFrom(desc.inputRate);
    vbDesc.instance_step_rate = desc.instanceStepRate;
    vbDesc.pitch = desc.pitch;
    vbDesc.slot = desc.slot;
    vbDescs.push_back(vbDesc);
  }
  pipelineCI.vertex_input_state.vertex_buffer_descriptions = vbDescs.data();

  auto *pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipelineCI);
  return makePtr<GraphicsPipeline>(createInfo.allocator, createInfo,
                                   downCast<Device>(getPtr()), pipeline);
}

Ptr<rhi::ComputePipeline>
Device::createComputePipeline(const ComputePipeline::CreateInfo &createInfo) {
  return makePtr<ComputePipeline>(createInfo.allocator, createInfo,
                                  downCast<Device>(getPtr()), nullptr);
}

void Device::submitCommandBuffer(Ptr<rhi::CommandBuffer> commandBuffer) {
  SDL_SubmitGPUCommandBuffer(
      downCast<CommandBuffer>(commandBuffer)->getNative());
}

Ptr<rhi::Texture>
Device::acquireSwapchainTexture(Ptr<rhi::CommandBuffer> commandBuffer) {
  auto raw = downCast<CommandBuffer>(commandBuffer);
  auto buffer = raw->getNative();
  if (buffer == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Command buffer is not valid for swapchain texture");
    return nullptr;
  }
  SDL_GPUTexture *nativeTex = nullptr;
  SDL_WaitAndAcquireGPUSwapchainTexture(buffer, window, &nativeTex, nullptr,
                                        nullptr);
  if (nativeTex == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
    return nullptr;
  }

  Texture::CreateInfo ci{};
  ci.allocator = commandBuffer->getCreateInfo().allocator;
  auto texture = makePtr<Texture>(commandBuffer->getCreateInfo().allocator, ci,
                                  downCast<Device>(getPtr()), nativeTex, true);
  return texture;
}

rhi::TextureFormat Device::getSwapchainFormat() const {
  auto format = SDL_GetGPUSwapchainTextureFormat(device, window);
  switch (format) {
  case SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM:
    return rhi::TextureFormat::B8G8R8A8_UNORM;
  case SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM:
    return rhi::TextureFormat::R8G8B8A8_UNORM;
  default:
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Unsupported swapchain format: %d", format);
    SDL_assert(false && "Unsupported swapchain format");
    return rhi::TextureFormat::Invalid;
  };
}

void Device::waitForGpuIdle() { SDL_WaitForGPUIdle(device); }

String Device::getDriver() const {
  return String(SDL_GetGPUDeviceDriver(device), getCreateInfo().allocator);
}
} // namespace sinen::rhi::sdlgpu
#endif // EMSCRIPTEN
