#include <graphics/rhi/rhi.hpp>
#include <memory>
#ifndef EMSCRIPTEN
#include "sdlgpu_backend.hpp"
#include "sdlgpu_convert.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include <iostream>

namespace sinen::rhi::sdlgpu {
Ptr<px::Device> Backend::createDevice(const Device::CreateInfo &createInfo) {
  SDL_GPUDevice *device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV,
                                              createInfo.debugMode, nullptr);
  if (!device) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Failed to create SDL_GPUDevice:\n");
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, SDL_GetError());
    return nullptr;
  }
  return makePtr<Device>(createInfo.allocator, createInfo, device);
}
void CopyPass::uploadTexture(const TextureTransferInfo &src,
                             const TextureRegion &dst, bool cycle) {
  SDL_GPUTextureTransferInfo transferInfo = {
      .transfer_buffer =
          downCast<TransferBuffer>(src.transferBuffer)->getNative(),
      .offset = src.offset,
  };
  SDL_GPUTextureRegion region = {
      .texture = downCast<Texture>(dst.texture)->getNative(),
      .mip_level = dst.mipLevel,
      .layer = dst.layer,
      .x = dst.x,
      .y = dst.y,
      .z = dst.z,
      .w = dst.width,
      .h = dst.height,
      .d = dst.depth,
  };
  SDL_UploadToGPUTexture(this->copyPass, &transferInfo, &region, cycle);
}
void CopyPass::downloadTexture(const TextureRegion &src,
                               const TextureTransferInfo &dst) {

  SDL_GPUTextureTransferInfo transferInfo = {
      .transfer_buffer =
          downCast<TransferBuffer>(dst.transferBuffer)->getNative(),
      .offset = dst.offset,
  };
  SDL_GPUTextureRegion region = {

      .texture = downCast<Texture>(src.texture)->getNative(),
      .x = src.x,
      .y = src.y,
      .z = src.z,
      .w = src.width,
      .h = src.height,
      .d = src.depth,
  };
  SDL_DownloadFromGPUTexture(this->copyPass, &region, &transferInfo);
}
void CopyPass::uploadBuffer(const BufferTransferInfo &src,
                            const BufferRegion &dst, bool cycle) {
  SDL_GPUTransferBufferLocation transferInfo = {
      .transfer_buffer =
          downCast<TransferBuffer>(src.transferBuffer)->getNative(),
      .offset = src.offset};
  SDL_GPUBufferRegion region = {.buffer =
                                    downCast<Buffer>(dst.buffer)->getNative(),
                                .offset = dst.offset,
                                .size = dst.size};
  SDL_UploadToGPUBuffer(this->copyPass, &transferInfo, &region, cycle);
}
void CopyPass::downloadBuffer(const BufferRegion &src,
                              const BufferTransferInfo &dst) {
  SDL_GPUBufferRegion region = {.buffer =
                                    downCast<Buffer>(src.buffer)->getNative(),
                                .offset = src.offset,
                                .size = src.size};
  SDL_GPUTransferBufferLocation transferInfo = {
      .transfer_buffer =
          downCast<TransferBuffer>(dst.transferBuffer)->getNative(),
      .offset = dst.offset,
  };
  SDL_DownloadFromGPUBuffer(this->copyPass, &region, &transferInfo);
}
void CopyPass::copyTexture(const TextureLocation &src,
                           const TextureLocation &dst, uint32 width,
                           uint32 height, uint32 depth, bool cycle) {
  SDL_GPUTextureLocation srcLocation = {
      .texture = downCast<Texture>(src.texture)->getNative(),
      .mip_level = src.mipLevel,
      .layer = src.layer,
      .x = src.x,
      .y = src.y,
      .z = src.z,
  };
  SDL_GPUTextureLocation dstLocation = {

      .texture = downCast<Texture>(dst.texture)->getNative(),
      .mip_level = dst.mipLevel,
      .layer = dst.layer,
      .x = dst.x,
      .y = dst.y,
      .z = dst.z,
  };
  SDL_CopyGPUTextureToTexture(this->copyPass, &srcLocation, &dstLocation, width,
                              height, depth, cycle);
}
void RenderPass::bindGraphicsPipeline(Ptr<px::GraphicsPipeline> pipeline) {
  SDL_BindGPUGraphicsPipeline(
      this->renderPass, downCast<GraphicsPipeline>(pipeline)->getNative());
}
void RenderPass::bindVertexBuffers(uint32 startSlot,
                                   const Array<BufferBinding> &bindings) {
  Array<SDL_GPUBufferBinding> bufferBindings(allocator);
  bufferBindings.resize(bindings.size());
  for (int i = 0; i < bindings.size(); ++i) {
    bufferBindings[i] = {};
    bufferBindings[i].buffer =
        downCast<Buffer>(bindings[i].buffer)->getNative();
    bufferBindings[i].offset = bindings[i].offset;
  }
  SDL_BindGPUVertexBuffers(this->renderPass, startSlot, bufferBindings.data(),
                           bufferBindings.size());
}
void RenderPass::bindIndexBuffer(const BufferBinding &binding,
                                 IndexElementSize indexElementSize) {
  SDL_GPUBufferBinding bufferBinding = {};
  bufferBinding.buffer = downCast<Buffer>(binding.buffer)->getNative();
  bufferBinding.offset = binding.offset;
  switch (indexElementSize) {
  case IndexElementSize::Uint16:
    SDL_BindGPUIndexBuffer(
        this->renderPass, &bufferBinding,
        SDL_GPUIndexElementSize::SDL_GPU_INDEXELEMENTSIZE_16BIT);
    break;
  case IndexElementSize::Uint32:
    SDL_BindGPUIndexBuffer(
        this->renderPass, &bufferBinding,
        SDL_GPUIndexElementSize::SDL_GPU_INDEXELEMENTSIZE_32BIT);
    break;
  default:
    assert(false && "Invalid index element size");
  }
}
void RenderPass::bindFragmentSamplers(
    uint32 startSlot, const Array<TextureSamplerBinding> &bindings) {
  Array<SDL_GPUTextureSamplerBinding> samplerBindings(allocator);
  samplerBindings.resize(bindings.size());
  for (int i = 0; i < samplerBindings.size(); ++i) {
    samplerBindings[i] = {};
    samplerBindings[i].sampler =
        downCast<Sampler>(bindings[i].sampler)->getNative();
    samplerBindings[i].texture =
        downCast<Texture>(bindings[i].texture)->getNative();
  }
  SDL_BindGPUFragmentSamplers(this->renderPass, startSlot,
                              samplerBindings.data(), samplerBindings.size());
}
void RenderPass::setViewport(const Viewport &viewport) {
  SDL_GPUViewport vp = {viewport.x,      viewport.y,        viewport.width,
                        viewport.height, viewport.minDepth, viewport.maxDepth};
  SDL_SetGPUViewport(this->renderPass, &vp);
}
void RenderPass::setScissor(int32 x, int32 y, int32 width, int32 height) {
  SDL_Rect rect = {x, y, width, height};
  SDL_SetGPUScissor(this->renderPass, &rect);
}
void RenderPass::drawPrimitives(uint32 vertexCount, uint32 instanceCount,
                                uint32 firstVertex, uint32 firstInstance) {
  SDL_DrawGPUPrimitives(this->renderPass, vertexCount, instanceCount,
                        firstVertex, firstInstance);
}
void RenderPass::drawIndexedPrimitives(uint32 indexCount, uint32 instanceCount,
                                       uint32 firstIndex, uint32 vertexOffset,
                                       uint32 firstInstance) {
  SDL_DrawGPUIndexedPrimitives(this->renderPass, indexCount, instanceCount,
                               firstIndex, vertexOffset, firstInstance);
}
Ptr<px::CopyPass> CommandBuffer::beginCopyPass() {
  auto *pass = SDL_BeginGPUCopyPass(this->commandBuffer);
  return makePtr<CopyPass>(getCreateInfo().allocator, getCreateInfo().allocator,
                           *this, pass);
}
void CommandBuffer::endCopyPass(Ptr<px::CopyPass> copyPass) {
  SDL_EndGPUCopyPass(downCast<CopyPass>(copyPass)->getNative());
}
Ptr<px::RenderPass>
CommandBuffer::beginRenderPass(const Array<ColorTargetInfo> &infos,
                               const DepthStencilTargetInfo &depthStencilInfo,
                               float r, float g, float b, float a) {
  Array<SDL_GPUColorTargetInfo> colorTargetInfos(getCreateInfo().allocator);
  colorTargetInfos.resize(infos.size());
  for (int i = 0; i < infos.size(); ++i) {
    colorTargetInfos[i] = {};
    colorTargetInfos[i].texture =
        downCast<Texture>(infos[i].texture)->getNative();
    colorTargetInfos[i].load_op = convert::LoadOpFrom(infos[i].loadOp);
    colorTargetInfos[i].store_op = convert::StoreOpFrom(infos[i].storeOp);
    colorTargetInfos[i].clear_color = {r, g, b, a};
  }
  SDL_GPUDepthStencilTargetInfo depthStencilTarget{};
  if (depthStencilInfo.texture != nullptr) {
    depthStencilTarget.texture =
        downCast<Texture>(depthStencilInfo.texture)->getNative();
    depthStencilTarget.clear_depth = depthStencilInfo.clearDepth;
    depthStencilTarget.load_op = convert::LoadOpFrom(depthStencilInfo.loadOp);
    depthStencilTarget.store_op =
        convert::StoreOpFrom(depthStencilInfo.storeOp);
    depthStencilTarget.stencil_load_op =
        convert::LoadOpFrom(depthStencilInfo.stencilLoadOp);
    depthStencilTarget.stencil_store_op =
        convert::StoreOpFrom(depthStencilInfo.stencilStoreOp);
    depthStencilTarget.cycle = depthStencilInfo.cycle;
    depthStencilTarget.clear_stencil = depthStencilInfo.clearStencil;
  }

  auto *renderPass = SDL_BeginGPURenderPass(
      commandBuffer, colorTargetInfos.data(), colorTargetInfos.size(),
      depthStencilInfo.texture ? &depthStencilTarget : nullptr);
  return makePtr<RenderPass>(getCreateInfo().allocator,
                             getCreateInfo().allocator, *this, renderPass);
}
void CommandBuffer::endRenderPass(Ptr<px::RenderPass> renderPass) {
  SDL_EndGPURenderPass(downCast<RenderPass>(renderPass)->getNative());
}
void CommandBuffer::pushUniformData(uint32 slot, const void *data,
                                    size_t size) {
  SDL_PushGPUVertexUniformData(this->commandBuffer, slot, data, size);
  SDL_PushGPUFragmentUniformData(this->commandBuffer, slot, data, size);
}
GraphicsPipeline::~GraphicsPipeline() {
  SDL_ReleaseGPUGraphicsPipeline(device->getNative(), pipeline);
}
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

Ptr<px::TransferBuffer>
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

Ptr<px::Buffer> Device::createBuffer(const Buffer::CreateInfo &createInfo) {
  SDL_GPUBufferCreateInfo bufferCI{};
  bufferCI.usage = convert::BufferUsageFrom(createInfo.usage);
  bufferCI.size = createInfo.size;
  SDL_GPUBuffer *buffer = SDL_CreateGPUBuffer(device, &bufferCI);
  return makePtr<Buffer>(createInfo.allocator, createInfo,
                         downCast<Device>(getPtr()), buffer);
}

Ptr<px::Texture> Device::createTexture(const Texture::CreateInfo &createInfo) {
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
Ptr<px::Sampler> Device::createSampler(const Sampler::CreateInfo &createInfo) {
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

TransferBuffer::~TransferBuffer() {
  SDL_ReleaseGPUTransferBuffer(device->getNative(), transferBuffer);
}

void *TransferBuffer::map(bool cycle) {
  return SDL_MapGPUTransferBuffer(device->getNative(), this->transferBuffer,
                                  cycle);
}
void TransferBuffer::unmap() {
  SDL_UnmapGPUTransferBuffer(device->getNative(), this->transferBuffer);
}

Buffer::~Buffer() { SDL_ReleaseGPUBuffer(device->getNative(), buffer); }

Ptr<px::Shader> Device::createShader(const Shader::CreateInfo &createInfo) {
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

  auto pD = (getPtr());
  auto p = downCast<Device>(pD);

  return makePtr<Shader>(createInfo.allocator, createInfo, p, shader);
}
Ptr<px::CommandBuffer>
Device::acquireCommandBuffer(const CommandBuffer::CreateInfo &createInfo) {
  SDL_GPUCommandBuffer *commandBuffer = SDL_AcquireGPUCommandBuffer(device);
  return makePtr<CommandBuffer>(createInfo.allocator, createInfo,
                                commandBuffer);
}

Ptr<px::GraphicsPipeline>
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
Ptr<px::ComputePipeline>
Device::createComputePipeline(const ComputePipeline::CreateInfo &createInfo) {
  return makePtr<ComputePipeline>(createInfo.allocator, createInfo,
                                  downCast<Device>(getPtr()), nullptr);
}
void Device::submitCommandBuffer(Ptr<px::CommandBuffer> commandBuffer) {
  SDL_SubmitGPUCommandBuffer(
      downCast<CommandBuffer>(commandBuffer)->getNative());
}
Ptr<px::Texture>
Device::acquireSwapchainTexture(Ptr<px::CommandBuffer> commandBuffer) {

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
px::TextureFormat Device::getSwapchainFormat() const {
  auto format = SDL_GetGPUSwapchainTextureFormat(device, window);
  switch (format) {
  case SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM:
    return px::TextureFormat::B8G8R8A8_UNORM;
  case SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM:
    return px::TextureFormat::R8G8B8A8_UNORM;
  default:
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Unsupported swapchain format: %d", format);
    SDL_assert(false && "Unsupported swapchain format");
    return px::TextureFormat::Invalid;
  };
}
void Device::waitForGpuIdle() { SDL_WaitForGPUIdle(device); }
String Device::getDriver() const {
  return String(SDL_GetGPUDeviceDriver(device), getCreateInfo().allocator);
}
Texture::~Texture() {
  if (!isSwapchainTexture)
    SDL_ReleaseGPUTexture(device->getNative(), texture);
}

Shader::~Shader() { SDL_ReleaseGPUShader(device->getNative(), shader); }
Sampler::~Sampler() { SDL_ReleaseGPUSampler(device->getNative(), sampler); }
} // namespace sinen::rhi::sdlgpu
#endif // EMSCRIPTEN