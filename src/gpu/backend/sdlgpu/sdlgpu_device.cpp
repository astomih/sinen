#include "sdlgpu_device.hpp"

#include <core/logger/log.hpp>

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
#include <cstring>

namespace sinen::gpu::sdlgpu {
namespace {
SDL_GPUShaderFormat shaderFormatFrom(ShaderFormat format) {
  switch (format) {
  case ShaderFormat::SPIRV:
    return SDL_GPU_SHADERFORMAT_SPIRV;
  case ShaderFormat::DXBC:
    return SDL_GPU_SHADERFORMAT_DXBC;
  case ShaderFormat::DXIL:
    return SDL_GPU_SHADERFORMAT_DXIL;
  default:
    return SDL_GPU_SHADERFORMAT_INVALID;
  }
}
} // namespace

Device::~Device() { releaseResources(); }

void Device::releaseResources() {
  if (window && device) {
    SDL_ReleaseWindowFromGPUDevice(device, window);
    window = nullptr;
  }
  if (device) {
    SDL_DestroyGPUDevice(device);
    device = nullptr;
  }
}

void Device::claimWindow(void *window) {
  if (!SDL_ClaimWindowForGPUDevice(device, static_cast<SDL_Window *>(window))) {
    Log::error("Failed to claim window for GPU device: {}", SDL_GetError());
  }
  this->window = static_cast<SDL_Window *>(window);
}

Ptr<gpu::TransferBuffer>
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

Ptr<gpu::Buffer> Device::createBuffer(const Buffer::CreateInfo &createInfo) {
  SDL_GPUBufferCreateInfo bufferCI{};
  bufferCI.usage = convert::BufferUsageFrom(createInfo.usage);
  bufferCI.size = createInfo.size;
  SDL_GPUBuffer *buffer = SDL_CreateGPUBuffer(device, &bufferCI);
  return makePtr<Buffer>(createInfo.allocator, createInfo,
                         downCast<Device>(getPtr()), buffer);
}

Ptr<gpu::Texture> Device::createTexture(const Texture::CreateInfo &createInfo) {
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

Ptr<gpu::Sampler> Device::createSampler(const Sampler::CreateInfo &createInfo) {
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

Ptr<gpu::Shader> Device::createShader(const Shader::CreateInfo &createInfo) {
  const SDL_GPUShaderFormat nativeFormat = shaderFormatFrom(createInfo.format);
  if (nativeFormat == SDL_GPU_SHADERFORMAT_INVALID ||
      (shaderFormats & nativeFormat) == 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "SDL_GPU backend does not support this shader format");
    return nullptr;
  }

  Array<Uint8> bytecode(createInfo.allocator);
  bytecode.resize(createInfo.size);
  std::memcpy(bytecode.data(), createInfo.data, createInfo.size);

  if (createInfo.stage == ShaderStage::Compute) {
    return makePtr<Shader>(createInfo.allocator, createInfo,
                           downCast<Device>(getPtr()), nullptr,
                           std::move(bytecode));
  }

  SDL_GPUShaderCreateInfo shaderCI = {};
  shaderCI.stage =
      createInfo.stage == ShaderStage::Vertex     ? SDL_GPU_SHADERSTAGE_VERTEX
      : createInfo.stage == ShaderStage::Fragment ? SDL_GPU_SHADERSTAGE_FRAGMENT
                                                  : SDL_GPU_SHADERSTAGE_VERTEX;
  shaderCI.code_size = createInfo.size;
  shaderCI.code = reinterpret_cast<const Uint8 *>(createInfo.data);
  shaderCI.format = nativeFormat;
  shaderCI.entrypoint = createInfo.entrypoint;
  shaderCI.num_samplers = createInfo.numSamplers;
  shaderCI.num_storage_buffers = createInfo.numStorageBuffers;
  shaderCI.num_storage_textures = createInfo.numStorageTextures;
  shaderCI.num_uniform_buffers = createInfo.numUniformBuffers;

  auto *shader = SDL_CreateGPUShader(device, &shaderCI);

  auto p = downCast<Device>(getPtr());

  return makePtr<Shader>(createInfo.allocator, createInfo, p, shader,
                         std::move(bytecode));
}

Ptr<gpu::CommandBuffer>
Device::acquireCommandBuffer(const CommandBuffer::CreateInfo &createInfo) {
  SDL_GPUCommandBuffer *commandBuffer = SDL_AcquireGPUCommandBuffer(device);
  return makePtr<CommandBuffer>(createInfo.allocator, createInfo,
                                commandBuffer);
}

Ptr<gpu::GraphicsPipeline>
Device::createGraphicsPipeline(const GraphicsPipeline::CreateInfo &createInfo) {
  SDL_GPUGraphicsPipelineCreateInfo pipelineCI = {};
  pipelineCI.vertex_shader =
      downCast<Shader>(createInfo.vertexShader)->getNative();
  pipelineCI.fragment_shader =
      downCast<Shader>(createInfo.fragmentShader)->getNative();
  {
    auto &rasterizerState = createInfo.rasterizerState;
    auto &sdlRasterizerState = pipelineCI.rasterizer_state;
    sdlRasterizerState.fill_mode =
        convert::FillModeFrom(createInfo.rasterizerState.fillMode);
    sdlRasterizerState.cull_mode =
        convert::CullModeFrom(createInfo.rasterizerState.cullMode);
    sdlRasterizerState.front_face =
        convert::FrontFaceFrom(createInfo.rasterizerState.frontFace);
    sdlRasterizerState.depth_bias_constant_factor =
        rasterizerState.depthBiasConstantFactor;
    sdlRasterizerState.depth_bias_clamp = rasterizerState.depthBiasClamp;
    sdlRasterizerState.depth_bias_slope_factor =
        rasterizerState.depthBiasSlopeFactor;
    sdlRasterizerState.enable_depth_bias = rasterizerState.enableDepthBias;
    sdlRasterizerState.enable_depth_clip = rasterizerState.enableDepthClip;
  }
  {
    auto &sdlMultisampleState = pipelineCI.multisample_state;
    auto &multiSampleState = createInfo.multiSampleState;
    sdlMultisampleState.enable_mask = multiSampleState.enableMask;
    sdlMultisampleState.sample_count =
        convert::SampleCountFrom(multiSampleState.sampleCount);
    sdlMultisampleState.sample_mask = multiSampleState.sampleMask;
  }
  {
    auto &sdlDepthStencilState = pipelineCI.depth_stencil_state;
    auto &depthStencilState = createInfo.depthStencilState;
    sdlDepthStencilState.compare_op =
        convert::CompareOpFrom(depthStencilState.compareOp);

    sdlDepthStencilState.back_stencil_state.fail_op =
        convert::StencilOpFrom(depthStencilState.backStencilState.failOp);
    sdlDepthStencilState.back_stencil_state.pass_op =
        convert::StencilOpFrom(depthStencilState.backStencilState.passOp);
    sdlDepthStencilState.back_stencil_state.depth_fail_op =
        convert::StencilOpFrom(depthStencilState.backStencilState.depthFailOp);
    sdlDepthStencilState.back_stencil_state.compare_op =
        convert::CompareOpFrom(depthStencilState.backStencilState.compareOp);

    sdlDepthStencilState.front_stencil_state.fail_op =
        convert::StencilOpFrom(depthStencilState.frontStencilState.failOp);
    sdlDepthStencilState.front_stencil_state.pass_op =
        convert::StencilOpFrom(depthStencilState.frontStencilState.passOp);
    sdlDepthStencilState.front_stencil_state.depth_fail_op =
        convert::StencilOpFrom(depthStencilState.frontStencilState.depthFailOp);
    sdlDepthStencilState.front_stencil_state.compare_op =
        convert::CompareOpFrom(depthStencilState.frontStencilState.compareOp);

    sdlDepthStencilState.compare_mask = depthStencilState.compareMask;
    sdlDepthStencilState.write_mask = depthStencilState.writeMask;
    sdlDepthStencilState.enable_depth_test = depthStencilState.enableDepthTest;
    sdlDepthStencilState.enable_depth_write =
        depthStencilState.enableDepthWrite;
    sdlDepthStencilState.enable_stencil_test =
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

Ptr<gpu::ComputePipeline>
Device::createComputePipeline(const ComputePipeline::CreateInfo &createInfo) {
  auto shader = downCast<Shader>(createInfo.computeShader);
  if (!shader) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "SDL_GPU: create compute pipeline missing shader");
    return nullptr;
  }
  const auto &shaderInfo = shader->getCreateInfo();
  const auto &bytecode = shader->getBytecode();
  const SDL_GPUShaderFormat nativeFormat = shaderFormatFrom(shaderInfo.format);
  SDL_GPUComputePipelineCreateInfo pipelineCI{};
  pipelineCI.code_size = bytecode.size();
  pipelineCI.code = bytecode.data();
  pipelineCI.entrypoint = shaderInfo.entrypoint;
  pipelineCI.format = nativeFormat;
  pipelineCI.num_samplers = shaderInfo.numSamplers;
  pipelineCI.num_readwrite_storage_buffers = shaderInfo.numStorageBuffers;
  pipelineCI.num_readwrite_storage_textures = shaderInfo.numStorageTextures;
  pipelineCI.num_uniform_buffers = shaderInfo.numUniformBuffers;
  pipelineCI.threadcount_x = createInfo.threadCountX;
  pipelineCI.threadcount_y = createInfo.threadCountY;
  pipelineCI.threadcount_z = createInfo.threadCountZ;
  auto *pipeline = SDL_CreateGPUComputePipeline(device, &pipelineCI);
  return makePtr<ComputePipeline>(createInfo.allocator, createInfo,
                                  downCast<Device>(getPtr()), pipeline);
}

void Device::submitCommandBuffer(Ptr<gpu::CommandBuffer> commandBuffer) {
  bool result = SDL_SubmitGPUCommandBuffer(
      downCast<CommandBuffer>(commandBuffer)->getNative());
  if (!result) {
    Log::error("Failed to submit command buffer: {}", SDL_GetError());
  }
}

Ptr<gpu::Texture>
Device::acquireSwapchainTexture(Ptr<gpu::CommandBuffer> commandBuffer) {
  auto raw = downCast<CommandBuffer>(commandBuffer);
  auto buffer = raw->getNative();
  if (buffer == nullptr) {
    Log::error("Command buffer is not valid for swapchain texture");
    return nullptr;
  }
  SDL_GPUTexture *nativeTex = nullptr;
  SDL_WaitAndAcquireGPUSwapchainTexture(buffer, window, &nativeTex, nullptr,
                                        nullptr);
  if (nativeTex == nullptr) {
    Log::error("Failed to acquire swapchain texture");
    return nullptr;
  }

  Texture::CreateInfo ci{};
  ci.allocator = commandBuffer->getCreateInfo().allocator;
  auto texture = makePtr<Texture>(commandBuffer->getCreateInfo().allocator, ci,
                                  downCast<Device>(getPtr()), nativeTex, true);
  return texture;
}

gpu::TextureFormat Device::getSwapchainFormat() const {
  auto format = SDL_GetGPUSwapchainTextureFormat(device, window);
  switch (format) {
  case SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM:
    return gpu::TextureFormat::B8G8R8A8_UNORM;
  case SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM:
    return gpu::TextureFormat::R8G8B8A8_UNORM;
  default:
    Log::error("Unsupported swapchain format: {}", static_cast<int>(format));
    SDL_assert(false && "Unsupported swapchain format");
    return gpu::TextureFormat::Invalid;
  };
}

void Device::waitForGpuIdle() { SDL_WaitForGPUIdle(device); }

String Device::getDriver() const {
  return String(SDL_GetGPUDeviceDriver(device), getCreateInfo().allocator);
}
} // namespace sinen::gpu::sdlgpu
#endif // EMSCRIPTEN
