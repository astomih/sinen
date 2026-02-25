#include "webgpu_device.hpp"
#include "webgpu_convert.hpp"

#include <core/def/macro.hpp>

#include <SDL3/SDL.h>
#include <SDL3/SDL_properties.h>
#include <SDL3/SDL_video.h>
#include <thread>
#include <vector>

namespace {
WGPUStringView toWgpuStringView(const char *str) {
  if (!str) {
    return {nullptr, 0};
  }
  return {str, WGPU_STRLEN};
}

struct QueueDoneState {
  bool done = false;
  bool success = false;
};

void onQueueDone(WGPUQueueWorkDoneStatus status, void *userdata1,
                 void *userdata2) {
  (void)userdata2;
  auto *state = static_cast<QueueDoneState *>(userdata1);
  state->done = true;
  state->success = (status == WGPUQueueWorkDoneStatus_Success);
}
} // namespace

namespace sinen::gpu::webgpu {
Device::~Device() {
  waitForGpuIdle();

  if (surface) {
    wgpuSurfaceUnconfigure(surface);
    wgpuSurfaceRelease(surface);
    surface = nullptr;
  }
  if (queue) {
    wgpuQueueRelease(queue);
    queue = nullptr;
  }
  if (device) {
    wgpuDeviceDestroy(device);
    wgpuDeviceRelease(device);
    device = nullptr;
  }
  if (adapter) {
    wgpuAdapterRelease(adapter);
    adapter = nullptr;
  }
  if (instance) {
    wgpuInstanceRelease(instance);
    instance = nullptr;
  }
}

bool Device::waitForFuture(WGPUFuture future) const {
  if (!instance || future.id == 0) {
    return false;
  }
  WGPUFutureWaitInfo waitInfo{};
  waitInfo.future = future;
  waitInfo.completed = false;

  while (true) {
    waitInfo.completed = false;
    const auto status = wgpuInstanceWaitAny(instance, 1, &waitInfo, 0);
    if (status == WGPUWaitStatus_Success && waitInfo.completed) {
      return true;
    }
    if (status == WGPUWaitStatus_TimedOut || status == WGPUWaitStatus_Success) {
      wgpuInstanceProcessEvents(instance);
      std::this_thread::yield();
      continue;
    }
    return false;
  }
}

WGPUTextureView
Device::createTextureView(WGPUTexture texture,
                          const gpu::Texture::CreateInfo &createInfo) const {
  if (!texture) {
    return nullptr;
  }
  WGPUTextureViewDescriptor desc{};
  desc.format = convert::TextureFormatFrom(createInfo.format);
  desc.dimension = convert::TextureViewDimensionFrom(createInfo.type);
  desc.baseMipLevel = 0;
  desc.mipLevelCount = createInfo.numLevels;
  desc.baseArrayLayer = 0;
  desc.arrayLayerCount = createInfo.layerCountOrDepth;
  desc.aspect = WGPUTextureAspect_All;
  desc.usage = convert::TextureUsageFrom(createInfo.usage);
  return wgpuTextureCreateView(texture, &desc);
}

WGPUTextureView Device::createDefaultTextureView(WGPUTexture texture) const {
  if (!texture) {
    return nullptr;
  }
  return wgpuTextureCreateView(texture, nullptr);
}

void Device::claimWindow(void *window) {
  this->window = static_cast<SDL_Window *>(window);

  if (!this->window) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "WebGPU claimWindow failed: window is null");
    return;
  }

  if (surface) {
    wgpuSurfaceUnconfigure(surface);
    wgpuSurfaceRelease(surface);
    surface = nullptr;
  }

#ifdef SINEN_PLATFORM_WINDOWS
  SDL_PropertiesID props = SDL_GetWindowProperties(this->window);
  void *hwnd = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER,
                                      nullptr);
  void *hinstance = SDL_GetPointerProperty(
      props, SDL_PROP_WINDOW_WIN32_INSTANCE_POINTER, nullptr);

  if (!hwnd || !hinstance) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "WebGPU claimWindow failed: missing HWND/HINSTANCE");
    return;
  }

  WGPUSurfaceSourceWindowsHWND hwndDesc{};
  hwndDesc.chain.sType = WGPUSType_SurfaceSourceWindowsHWND;
  hwndDesc.chain.next = nullptr;
  hwndDesc.hinstance = hinstance;
  hwndDesc.hwnd = hwnd;

  WGPUSurfaceDescriptor surfaceDesc{};
  surfaceDesc.nextInChain = &hwndDesc.chain;
  surfaceDesc.label = toWgpuStringView("sinen-webgpu-surface");
  surface = wgpuInstanceCreateSurface(instance, &surfaceDesc);
#else
  surface = nullptr;
#endif

  if (!surface) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Failed to create WebGPU surface");
    return;
  }

  configureSurfaceIfNeeded();
}

Ptr<gpu::Buffer> Device::createBuffer(const Buffer::CreateInfo &createInfo) {
  WGPUBufferDescriptor desc{};
  desc.label = toWgpuStringView("sinen-buffer");
  desc.usage = convert::BufferUsageFrom(createInfo.usage);
  desc.size = createInfo.size;
  desc.mappedAtCreation = false;

  auto native = wgpuDeviceCreateBuffer(device, &desc);
  return makePtr<Buffer>(createInfo.allocator, createInfo, get(), native);
}

Ptr<gpu::Texture> Device::createTexture(const Texture::CreateInfo &createInfo) {
  WGPUTextureDescriptor desc{};
  desc.label = toWgpuStringView("sinen-texture");
  desc.usage = convert::TextureUsageFrom(createInfo.usage);
  desc.dimension = convert::TextureDimensionFrom(createInfo.type);
  desc.size = {createInfo.width, createInfo.height,
               createInfo.layerCountOrDepth};
  desc.format = convert::TextureFormatFrom(createInfo.format);
  desc.mipLevelCount = createInfo.numLevels;
  desc.sampleCount = convert::SampleCountFrom(createInfo.sampleCount);
  desc.viewFormatCount = 0;
  desc.viewFormats = nullptr;

  auto texture = wgpuDeviceCreateTexture(device, &desc);
  auto view = createTextureView(texture, createInfo);
  return makePtr<Texture>(createInfo.allocator, createInfo, get(), texture,
                          view, false);
}

Ptr<gpu::Sampler> Device::createSampler(const Sampler::CreateInfo &createInfo) {
  WGPUSamplerDescriptor desc{};
  desc.addressModeU = convert::AddressModeFrom(createInfo.addressModeU);
  desc.addressModeV = convert::AddressModeFrom(createInfo.addressModeV);
  desc.addressModeW = convert::AddressModeFrom(createInfo.addressModeW);
  desc.magFilter = convert::FilterFrom(createInfo.magFilter);
  desc.minFilter = convert::FilterFrom(createInfo.minFilter);
  desc.mipmapFilter = convert::MipmapModeFrom(createInfo.mipmapMode);
  desc.lodMinClamp = createInfo.minLod;
  desc.lodMaxClamp = createInfo.maxLod;
  desc.compare = convert::CompareOpFrom(createInfo.compareOp);
  desc.maxAnisotropy = createInfo.enableAnisotropy
                           ? static_cast<uint16_t>(createInfo.maxAnisotropy)
                           : 1;

  auto sampler = wgpuDeviceCreateSampler(device, &desc);
  return makePtr<Sampler>(createInfo.allocator, createInfo, get(), sampler);
}

Ptr<gpu::TransferBuffer>
Device::createTransferBuffer(const TransferBuffer::CreateInfo &createInfo) {
  WGPUBuffer nativeBuffer = nullptr;
  if (createInfo.usage == TransferBufferUsage::Download) {
    WGPUBufferDescriptor desc{};
    desc.label = toWgpuStringView("sinen-transfer-download");
    desc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_MapRead;
    desc.size = createInfo.size;
    desc.mappedAtCreation = false;
    nativeBuffer = wgpuDeviceCreateBuffer(device, &desc);
  }
  return makePtr<TransferBuffer>(createInfo.allocator, createInfo, get(),
                                 nativeBuffer);
}

Ptr<gpu::Shader> Device::createShader(const Shader::CreateInfo &createInfo) {
  WGPUShaderModuleDescriptor shaderDesc{};
  shaderDesc.label = toWgpuStringView("sinen-shader");
  WGPUShaderSourceSPIRV spirvDesc{};
  WGPUShaderSourceWGSL wgslDesc{};

  switch (createInfo.format) {
  case ShaderFormat::SPIRV:
    spirvDesc.chain.sType = WGPUSType_ShaderSourceSPIRV;
    spirvDesc.chain.next = nullptr;
    spirvDesc.code = static_cast<const uint32_t *>(createInfo.data);
    spirvDesc.codeSize =
        static_cast<uint32_t>(createInfo.size / sizeof(uint32_t));
    shaderDesc.nextInChain = &spirvDesc.chain;
    break;
  case ShaderFormat::WGSL:
    wgslDesc.chain.sType = WGPUSType_ShaderSourceWGSL;
    wgslDesc.chain.next = nullptr;
    wgslDesc.code = toWgpuStringView(static_cast<const char *>(createInfo.data));
    shaderDesc.nextInChain = &wgslDesc.chain;
    break;
  default:
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "WebGPU backend does not support shader format: %d",
                 static_cast<int>(createInfo.format));
    return nullptr;
  }

  auto shader = wgpuDeviceCreateShaderModule(device, &shaderDesc);
  return makePtr<Shader>(createInfo.allocator, createInfo, get(), shader);
}

Ptr<gpu::CommandBuffer>
Device::acquireCommandBuffer(const CommandBuffer::CreateInfo &createInfo) {
  WGPUCommandEncoderDescriptor desc{};
  desc.label = toWgpuStringView("sinen-command-encoder");
  auto encoder = wgpuDeviceCreateCommandEncoder(device, &desc);
  return makePtr<CommandBuffer>(createInfo.allocator, createInfo, get(),
                                encoder);
}

Ptr<gpu::GraphicsPipeline>
Device::createGraphicsPipeline(const GraphicsPipeline::CreateInfo &createInfo) {
  auto vs = downCast<Shader>(createInfo.vertexShader);
  auto fs = downCast<Shader>(createInfo.fragmentShader);
  if (!vs || !fs) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "createGraphicsPipeline failed: shader cast failed");
    return nullptr;
  }

  const int vbCount =
      createInfo.vertexInputState.vertexBufferDescriptions.size();
  std::vector<WGPUVertexBufferLayout> vertexBuffers(vbCount);
  std::vector<std::vector<WGPUVertexAttribute>> attributesByBuffer(vbCount);

  for (int i = 0; i < createInfo.vertexInputState.vertexAttributes.size();
       ++i) {
    const auto &attr = createInfo.vertexInputState.vertexAttributes[i];
    int vbIndex = -1;
    for (int j = 0; j < vbCount; ++j) {
      if (createInfo.vertexInputState.vertexBufferDescriptions[j].slot ==
          attr.bufferSlot) {
        vbIndex = j;
        break;
      }
    }
    if (vbIndex < 0) {
      continue;
    }

    WGPUVertexAttribute nativeAttr{};
    nativeAttr.shaderLocation = attr.location;
    nativeAttr.offset = attr.offset;
    nativeAttr.format = convert::VertexElementFormatFrom(attr.format);
    attributesByBuffer[vbIndex].push_back(nativeAttr);
  }

  for (int i = 0; i < vbCount; ++i) {
    const auto &vbDesc =
        createInfo.vertexInputState.vertexBufferDescriptions[i];
    vertexBuffers[i] = {};
    vertexBuffers[i].stepMode = convert::VertexInputRateFrom(vbDesc.inputRate);
    vertexBuffers[i].arrayStride = vbDesc.pitch;
    vertexBuffers[i].attributeCount = attributesByBuffer[i].size();
    vertexBuffers[i].attributes = attributesByBuffer[i].data();
  }

  const int colorTargetCount =
      createInfo.targetInfo.colorTargetDescriptions.size();
  std::vector<WGPUColorTargetState> colorTargets(colorTargetCount);
  std::vector<WGPUBlendState> blendStates(colorTargetCount);

  for (int i = 0; i < colorTargetCount; ++i) {
    const auto &target = createInfo.targetInfo.colorTargetDescriptions[i];
    colorTargets[i] = {};
    colorTargets[i].format = convert::TextureFormatFrom(target.format);
    colorTargets[i].writeMask =
        target.blendState.enableColorWriteMask
            ? convert::ColorWriteMaskFrom(target.blendState.colorWriteMask)
            : WGPUColorWriteMask_All;

    if (target.blendState.enableBlend) {
      blendStates[i] = {};
      blendStates[i].color.operation =
          convert::BlendOpFrom(target.blendState.colorBlendOp);
      blendStates[i].color.srcFactor =
          convert::BlendFactorFrom(target.blendState.srcColorBlendFactor);
      blendStates[i].color.dstFactor =
          convert::BlendFactorFrom(target.blendState.dstColorBlendFactor);
      blendStates[i].alpha.operation =
          convert::BlendOpFrom(target.blendState.alphaBlendOp);
      blendStates[i].alpha.srcFactor =
          convert::BlendFactorFrom(target.blendState.srcAlphaBlendFactor);
      blendStates[i].alpha.dstFactor =
          convert::BlendFactorFrom(target.blendState.dstAlphaBlendFactor);
      colorTargets[i].blend = &blendStates[i];
    } else {
      colorTargets[i].blend = nullptr;
    }
  }

  WGPUVertexState vertexState{};
  vertexState.module = vs->getNative();
  vertexState.entryPoint = toWgpuStringView(vs->getEntryPoint());
  vertexState.constantCount = 0;
  vertexState.constants = nullptr;
  vertexState.bufferCount = vertexBuffers.size();
  vertexState.buffers = vertexBuffers.data();

  WGPUFragmentState fragmentState{};
  fragmentState.module = fs->getNative();
  fragmentState.entryPoint = toWgpuStringView(fs->getEntryPoint());
  fragmentState.constantCount = 0;
  fragmentState.constants = nullptr;
  fragmentState.targetCount = colorTargets.size();
  fragmentState.targets = colorTargets.data();

  WGPUPrimitiveState primitiveState{};
  primitiveState.topology =
      convert::PrimitiveTypeFrom(createInfo.primitiveType);
  primitiveState.stripIndexFormat = WGPUIndexFormat_Undefined;
  primitiveState.frontFace =
      convert::FrontFaceFrom(createInfo.rasterizerState.frontFace);
  primitiveState.cullMode =
      convert::CullModeFrom(createInfo.rasterizerState.cullMode);
  primitiveState.unclippedDepth = !createInfo.rasterizerState.enableDepthClip;

  WGPUDepthStencilState depthStencilState{};
  WGPUDepthStencilState *depthStencilStatePtr = nullptr;
  if (createInfo.targetInfo.hasDepthStencilTarget) {
    depthStencilState = {};
    depthStencilState.format = convert::TextureFormatFrom(
        createInfo.targetInfo.depthStencilTargetFormat);
    depthStencilState.depthWriteEnabled = convert::OptionalBoolFrom(
        createInfo.depthStencilState.enableDepthWrite);
    depthStencilState.depthCompare =
        convert::CompareOpFrom(createInfo.depthStencilState.compareOp);
    depthStencilState.stencilFront.compare = convert::CompareOpFrom(
        createInfo.depthStencilState.frontStencilState.compareOp);
    depthStencilState.stencilFront.failOp = convert::StencilOpFrom(
        createInfo.depthStencilState.frontStencilState.failOp);
    depthStencilState.stencilFront.depthFailOp = convert::StencilOpFrom(
        createInfo.depthStencilState.frontStencilState.depthFailOp);
    depthStencilState.stencilFront.passOp = convert::StencilOpFrom(
        createInfo.depthStencilState.frontStencilState.passOp);
    depthStencilState.stencilBack.compare = convert::CompareOpFrom(
        createInfo.depthStencilState.backStencilState.compareOp);
    depthStencilState.stencilBack.failOp = convert::StencilOpFrom(
        createInfo.depthStencilState.backStencilState.failOp);
    depthStencilState.stencilBack.depthFailOp = convert::StencilOpFrom(
        createInfo.depthStencilState.backStencilState.depthFailOp);
    depthStencilState.stencilBack.passOp = convert::StencilOpFrom(
        createInfo.depthStencilState.backStencilState.passOp);
    depthStencilState.stencilReadMask =
        createInfo.depthStencilState.compareMask;
    depthStencilState.stencilWriteMask = createInfo.depthStencilState.writeMask;
    depthStencilState.depthBias = static_cast<int32_t>(
        createInfo.rasterizerState.depthBiasConstantFactor);
    depthStencilState.depthBiasSlopeScale =
        createInfo.rasterizerState.depthBiasSlopeFactor;
    depthStencilState.depthBiasClamp =
        createInfo.rasterizerState.depthBiasClamp;
    depthStencilStatePtr = &depthStencilState;
  }

  WGPUMultisampleState multisampleState{};
  multisampleState.count =
      convert::SampleCountFrom(createInfo.multiSampleState.sampleCount);
  multisampleState.mask = createInfo.multiSampleState.enableMask
                              ? createInfo.multiSampleState.sampleMask
                              : ~0u;
  multisampleState.alphaToCoverageEnabled = false;

  WGPURenderPipelineDescriptor pipelineDesc{};
  pipelineDesc.label = toWgpuStringView("sinen-render-pipeline");
  pipelineDesc.layout = nullptr; // auto layout
  pipelineDesc.vertex = vertexState;
  pipelineDesc.primitive = primitiveState;
  pipelineDesc.depthStencil = depthStencilStatePtr;
  pipelineDesc.multisample = multisampleState;
  pipelineDesc.fragment = &fragmentState;

  auto pipeline = wgpuDeviceCreateRenderPipeline(device, &pipelineDesc);
  return makePtr<GraphicsPipeline>(createInfo.allocator, createInfo, get(),
                                   pipeline);
}

Ptr<gpu::ComputePipeline>
Device::createComputePipeline(const ComputePipeline::CreateInfo &createInfo) {
  return makePtr<ComputePipeline>(createInfo.allocator, createInfo, get(),
                                  nullptr);
}

void Device::submitCommandBuffer(Ptr<gpu::CommandBuffer> commandBuffer) {
  auto cb = downCast<CommandBuffer>(commandBuffer);
  if (!cb) {
    return;
  }
  auto native = cb->finish();
  if (!native) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "submitCommandBuffer failed: native command buffer is null");
    return;
  }
  wgpuQueueSubmit(queue, 1, &native);
  cb->setSubmitted(true);

  if (cb->getShouldPresent() && surface) {
    wgpuSurfacePresent(surface);
  }
}

Ptr<gpu::Texture>
Device::acquireSwapchainTexture(Ptr<gpu::CommandBuffer> commandBuffer) {
  auto cb = downCast<CommandBuffer>(commandBuffer);
  if (!cb || !surface) {
    return nullptr;
  }

  configureSurfaceIfNeeded();

  WGPUSurfaceTexture surfaceTexture{};
  wgpuSurfaceGetCurrentTexture(surface, &surfaceTexture);

  if (surfaceTexture.status == WGPUSurfaceGetCurrentTextureStatus_Outdated ||
      surfaceTexture.status == WGPUSurfaceGetCurrentTextureStatus_Lost) {
    configureSurfaceIfNeeded();
    wgpuSurfaceGetCurrentTexture(surface, &surfaceTexture);
  }

  if ((surfaceTexture.status !=
           WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal &&
       surfaceTexture.status !=
           WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal) ||
      !surfaceTexture.texture) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "acquireSwapchainTexture failed with status: %d",
                 static_cast<int>(surfaceTexture.status));
    return nullptr;
  }

  Texture::CreateInfo ci{};
  ci.allocator = commandBuffer->getCreateInfo().allocator;
  ci.type = TextureType::Texture2D;
  ci.format = convert::TextureFormatTo(swapchainFormat);
  ci.usage = TextureUsage::ColorTarget;
  ci.width = configuredWidth;
  ci.height = configuredHeight;
  ci.layerCountOrDepth = 1;
  ci.numLevels = 1;
  ci.sampleCount = SampleCount::x1;

  auto view = createDefaultTextureView(surfaceTexture.texture);
  cb->setShouldPresent(true);
  return makePtr<Texture>(ci.allocator, ci, get(), surfaceTexture.texture, view,
                          true);
}

gpu::TextureFormat Device::getSwapchainFormat() const {
  return convert::TextureFormatTo(swapchainFormat);
}

void Device::waitForGpuIdle() {
  if (!queue) {
    return;
  }
  QueueDoneState state{};
  WGPUQueueWorkDoneCallbackInfo callbackInfo{};
  callbackInfo.mode = WGPUCallbackMode_WaitAnyOnly;
  callbackInfo.callback = &onQueueDone;
  callbackInfo.userdata1 = &state;
  callbackInfo.userdata2 = nullptr;

  auto future = wgpuQueueOnSubmittedWorkDone(queue, callbackInfo);
  waitForFuture(future);
}

String Device::getDriver() const {
  return String("webgpu", getCreateInfo().allocator);
}

void Device::configureSurfaceIfNeeded() {
  if (!window || !surface) {
    return;
  }

  int w = 0;
  int h = 0;
  SDL_GetWindowSizeInPixels(window, &w, &h);
  if (w <= 0 || h <= 0) {
    return;
  }

  if (configuredWidth != static_cast<UInt32>(w) ||
      configuredHeight != static_cast<UInt32>(h) ||
      swapchainFormat == WGPUTextureFormat_Undefined) {
    configureSurface(static_cast<UInt32>(w), static_cast<UInt32>(h));
  }
}

void Device::configureSurface(UInt32 width, UInt32 height) {
  if (!surface) {
    return;
  }

  WGPUSurfaceCapabilities caps{};
  if (wgpuSurfaceGetCapabilities(surface, adapter, &caps) !=
          WGPUStatus_Success ||
      caps.formatCount == 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Failed to query WebGPU surface capabilities");
    return;
  }

  WGPUTextureFormat chosenFormat = caps.formats[0];
  for (size_t i = 0; i < caps.formatCount; ++i) {
    if (caps.formats[i] == WGPUTextureFormat_BGRA8Unorm) {
      chosenFormat = caps.formats[i];
      break;
    }
    if (caps.formats[i] == WGPUTextureFormat_RGBA8Unorm) {
      chosenFormat = caps.formats[i];
    }
  }

  WGPUPresentMode presentMode = WGPUPresentMode_Fifo;
  if (caps.presentModeCount > 0) {
    presentMode = caps.presentModes[0];
    for (size_t i = 0; i < caps.presentModeCount; ++i) {
      if (caps.presentModes[i] == WGPUPresentMode_Fifo) {
        presentMode = WGPUPresentMode_Fifo;
        break;
      }
    }
  }

  WGPUCompositeAlphaMode alphaMode = caps.alphaModeCount > 0
                                         ? caps.alphaModes[0]
                                         : WGPUCompositeAlphaMode_Auto;

  WGPUSurfaceConfiguration config{};
  config.device = device;
  config.format = chosenFormat;
  config.usage = WGPUTextureUsage_RenderAttachment;
  config.width = width;
  config.height = height;
  config.viewFormatCount = 0;
  config.viewFormats = nullptr;
  config.alphaMode = alphaMode;
  config.presentMode = presentMode;

  wgpuSurfaceConfigure(surface, &config);

  swapchainFormat = chosenFormat;
  configuredWidth = width;
  configuredHeight = height;

  wgpuSurfaceCapabilitiesFreeMembers(caps);
}
} // namespace sinen::gpu::webgpu
