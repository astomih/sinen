#if 0
#ifndef __ANDROID__
#include <webgpu/webgpu.h>
#ifdef EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/html5.h>
#endif // EMSCRIPTEN
#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_system.h>

#include <imgui.h>

#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_wgpu.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "webgpu_renderer.hpp"
#include <iostream>

namespace paranoixa {
#define COUNT_OF(x)                                                            \
  ((sizeof(x) / sizeof(0 [x])) / ((size_t)(!(sizeof(x) % sizeof(0 [x])))))
WebGPURenderer::WebGPURenderer(Allocator *allocator)
    : instance(nullptr), adapter(nullptr), device(nullptr), queue(nullptr),
      surface(nullptr), targetView(nullptr), bindGroup(nullptr),
      surfaceFormat(WGPUTextureFormat_Undefined), texture{}, sampler(nullptr),
      pipeline(nullptr), vertexBuffer(nullptr), encoder(nullptr) {}
WebGPURenderer::~WebGPURenderer() {
  ImGui_ImplWGPU_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();
  wgpuRenderPipelineRelease(pipeline);
#ifndef __EMSCRIPTEN__
  if (surface) {
    wgpuSurfaceRelease(surface);
  }
#endif
  if (queue) {
    wgpuQueueRelease(queue);
  }
  if (device) {
    wgpuDeviceRelease(device);
  }
  if (adapter) {
    wgpuAdapterRelease(adapter);
  }
  if (instance) {
    wgpuInstanceRelease(instance);
  }
}
void WebGPURenderer::Initialize(void *window) {
  PrepareInstance();
  PrepareAdapter();
  PrepareDevice();
  PrepareQueue();
  PrepareSurface(window);
  int width, height;
  SDL_GetWindowSize(static_cast<SDL_Window *>(window), &width, &height);
  ConfigSurface(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
  // Load texture from SDL
  SDL_Surface *surface = SDL_LoadBMP("res/texture.bmp");

  std::vector<uint8_t> data;
  data.resize(surface->w * surface->h * 4);

  for (int y = 0; y < surface->h; ++y) {
    for (int x = 0; x < surface->w; ++x) {
      auto pixel =
          static_cast<uint32_t *>(surface->pixels) + y * surface->w + x;
      auto r = (*pixel & 0x00FF0000) >> 16;
      auto g = (*pixel & 0x0000FF00) >> 8;
      auto b = (*pixel & 0x000000FF);
      auto a = (*pixel & 0xFF000000) >> 24;
      auto index = (y * surface->w + x) * 4;
      data[index + 0] = r;
      data[index + 1] = g;
      data[index + 2] = b;
      data[index + 3] = a;
    }
  }
  texture = CreateTexture(data.data(), data.size(), surface->w, surface->h);
  SDL_DestroySurface(surface);
  PrepareSampler();

  InitializePipeline();
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
  io.IniFilename = nullptr;
  ImGui::StyleColorsDark();
  if (not ImGui_ImplSDL3_InitForOther(static_cast<SDL_Window *>(window)))
    return;
  ImGui_ImplWGPU_InitInfo init_info{};
  init_info.Device = device;
  init_info.RenderTargetFormat = surfaceFormat;
  init_info.NumFramesInFlight = 1;
  if (not ImGui_ImplWGPU_Init(&init_info))
    return;

  /*
 (-1,  1)  (1,  1)
    +--------+
    |        |
    |        |
    |        |
    +--------+
 (-1, -1)  (1, -1)
  */
  float quadVerts[] = {
      -1.f, -1.f, 0.f, 0, 0, 0, 0, 1, // position, uv, color
      -1.f, 1.f,  0.f, 0, 1, 1, 0, 0, //
      1.f,  -1.f, 0.f, 1, 0, 0, 0, 1, //
      1.f,  -1.f, 0.f, 1, 0, 0, 0, 1, //
      -1.f, 1.f,  0.f, 0, 1, 1, 0, 0, //
      1.f,  1.f,  0.f, 1, 1, 1, 0, 0, //
  };
  vertexBuffer = this->CreateBuffer(sizeof(float) * COUNT_OF(quadVerts),
                                    WGPUBufferUsage_Vertex);

  void *mapped = wgpuBufferGetMappedRange(vertexBuffer, 0,
                                          sizeof(float) * COUNT_OF(quadVerts));
  memcpy(mapped, quadVerts, sizeof(float) * COUNT_OF(quadVerts));
  wgpuBufferUnmap(vertexBuffer);
}
void WebGPURenderer::ProcessEvent(void *event) {
  ImGui_ImplSDL3_ProcessEvent(static_cast<SDL_Event *>(event));
}
#define PX_ASSERT(condition)                                                   \
  if (!(condition)) {                                                          \
    std::cerr << "Paranoixa Assertion failed: " << #condition << std::endl;    \
    assert(condition);                                                         \
  }
void WebGPURenderer::BeginFrame() {
  PX_ASSERT(surface)
  targetView = GetNextSurfaceTextureView();
  PX_ASSERT(targetView)

  // Create a command encoder
  WGPUCommandEncoderDescriptor encoderDesc{};
  encoderDesc.nextInChain = nullptr;
  encoder = wgpuDeviceCreateCommandEncoder(device, &encoderDesc);

  // Create the render pass that clear the screen with our color
  WGPURenderPassColorAttachment colorAttachment{};
  colorAttachment.view = targetView;
  colorAttachment.resolveTarget = nullptr;
  colorAttachment.loadOp = WGPULoadOp_Clear;
  colorAttachment.storeOp = WGPUStoreOp_Store;
  colorAttachment.clearValue =
      WGPUColor{.r = 1.0f, .g = 0.4f, .b = 0.0f, .a = 1.0f};
#ifndef WEBGPU_BACKEND_WGPU
  colorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
#endif

  WGPURenderPassDescriptor renderPassDesc{};
  renderPassDesc.nextInChain = nullptr;
  renderPassDesc.label = GetStringView("Paranoixa Render pass");
  renderPassDesc.colorAttachmentCount = 1;
  renderPassDesc.colorAttachments = &colorAttachment;
  renderPassDesc.depthStencilAttachment = nullptr;
  WGPURenderPassEncoder renderPass =
      wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);
  PX_ASSERT(renderPass);

  // Set the pipeline and draw
  wgpuRenderPassEncoderSetPipeline(renderPass, pipeline);
  wgpuRenderPassEncoderSetBindGroup(renderPass, 0, bindGroup, 0, nullptr);
  wgpuRenderPassEncoderSetVertexBuffer(renderPass, 0, vertexBuffer, 0, 192);
  wgpuRenderPassEncoderDraw(renderPass, 6, 1, 0, 0);

  ImGui_ImplSDL3_NewFrame();
  ImGui_ImplWGPU_NewFrame();
  ImGui::NewFrame();
  ImGui::ShowDemoWindow();
  ImGui::Begin("FPS");
  ImGuiIO &io = ImGui::GetIO();
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
              1000.0f / io.Framerate, io.Framerate);
  ImGui::End();
  ImGui::Render();
  ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), renderPass);
  ImGui::EndFrame();
  wgpuRenderPassEncoderEnd(renderPass);
}
void WebGPURenderer::EndFrame() {
  // Submit the command buffer
  WGPUCommandBufferDescriptor cmdBufferDesc{};
  cmdBufferDesc.nextInChain = nullptr;
  cmdBufferDesc.label = GetStringView("Paranoixa Command buffer");
  WGPUCommandBuffer cmdBuffer =
      wgpuCommandEncoderFinish(encoder, &cmdBufferDesc);

  wgpuCommandEncoderRelease(encoder);
  wgpuQueueSubmit(queue, 1, &cmdBuffer);
  wgpuCommandBufferRelease(cmdBuffer);
  wgpuTextureViewRelease(targetView);
#ifndef __EMSCRIPTEN__
  wgpuSurfacePresent(surface);
#endif
}
void WebGPURenderer::AddGuiUpdateCallBack(std::function<void()> callBack) {}
void WebGPURenderer::PrepareSurface(void *window) {
#ifdef __EMSCRIPTEN__
  WGPUSurfaceDescriptorFromCanvasHTMLSelector fromCanvasHTMLSelector = {};
  fromCanvasHTMLSelector.chain.next = NULL;
  fromCanvasHTMLSelector.chain.sType =
      WGPUSType_SurfaceDescriptorFromCanvasHTMLSelector;
  fromCanvasHTMLSelector.selector = "canvas";

  WGPUSurfaceDescriptor surfaceDescriptor = {};
  surfaceDescriptor.nextInChain = &fromCanvasHTMLSelector.chain;
  surfaceDescriptor.label = NULL;

  surface = wgpuInstanceCreateSurface(instance, &surfaceDescriptor);
#endif
#ifdef _WIN32
  {
    auto hwnd = static_cast<HWND>(SDL_GetPointerProperty(
        SDL_GetWindowProperties(static_cast<SDL_Window *>(window)),
        SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));
    HINSTANCE hinstance = GetModuleHandle(nullptr);
#ifdef DAWN
    WGPUSurfaceSourceWindowsHWND desc{
        .chain =
            WGPUChainedStruct{
                .next = nullptr,
                .sType = WGPUSType_SurfaceSourceWindowsHWND,
            },
        .hinstance = hinstance,
        .hwnd = hwnd};
#else
    WGPUSurfaceDescriptorFromWindowsHWND desc{
        .chain =
            WGPUChainedStruct{
                .next = nullptr,
                .sType = WGPUSType_SurfaceDescriptorFromWindowsHWND,
            },
        .hinstance = hinstance,
        .hwnd = hwnd};
#endif

    auto descriptor =
        WGPUSurfaceDescriptor{.nextInChain = &desc.chain, .label = nullptr};
    surface = wgpuInstanceCreateSurface(instance, &descriptor);
  }
#endif
}
void WebGPURenderer::PrepareInstance() {
#ifdef EMSCRIPTEN
  instance = wgpuCreateInstance(nullptr);
#else
  WGPUInstanceDescriptor desc = {};
  desc.nextInChain = nullptr;
  instance = wgpuCreateInstance(&desc);
#endif

  if (!instance) {
    std::cerr << "Could not initialize WebGPU!" << std::endl;
  }
}
void WebGPURenderer::PrepareAdapter() {
  struct UserData {
    WGPUAdapter adapter;
    bool adapterRequested = false;
  };
  WGPURequestAdapterOptions adapterOpts = {};
  adapterOpts.nextInChain = nullptr;
#ifdef DAWN
  auto onAdapterRequestEnded = [](WGPURequestAdapterStatus status,
                                  WGPUAdapter adapter, WGPUStringView message,
                                  void *pUserData) {
#else
  auto onAdapterRequestEnded = [](WGPURequestAdapterStatus status,
                                  WGPUAdapter adapter, const char *message,
                                  void *pUserData) {
#endif
    UserData &userData = *reinterpret_cast<UserData *>(pUserData);
    userData.adapterRequested = true;
    if (status == WGPURequestAdapterStatus_Success) {
      userData.adapter = adapter;
    } else {
      std::cout << "Could not get WebGPU adapter: ";
#ifdef DAWN
      if (message.data) {
        std::cout << message.data;
      }
#else
      std::cout << message;
#endif
    }
  };
  UserData userData{};
  wgpuInstanceRequestAdapter(instance, nullptr, onAdapterRequestEnded,
                             &userData);
#ifdef __EMSCRIPTEN__
  while (userData.adapterRequested == false) {
    emscripten_sleep(100);
  }
#endif // __EMSCRIPTEN__
  adapter = userData.adapter;
}
void WebGPURenderer::PrepareDevice() {
  struct UserData {
    WGPUDevice device = nullptr;
    bool requestEnded = false;
  };
  UserData userData;

#ifdef DAWN
  auto onDeviceRequestEnded = [](WGPURequestDeviceStatus status,
                                 WGPUDevice device, WGPUStringView message,
                                 void *pUserData) {
    UserData &userData = *reinterpret_cast<UserData *>(pUserData);
    if (status == WGPURequestDeviceStatus_Success) {
      userData.device = device;
    } else {
      std::cout << "Could not get WebGPU device: " << message.data << std::endl;
    }
    userData.requestEnded = true;
  };
  WGPUDeviceDescriptor descriptor{};
  descriptor.deviceLostCallbackInfo.mode = WGPUCallbackMode_AllowProcessEvents;
  descriptor.deviceLostCallbackInfo.callback =
      [](const WGPUDevice *device, WGPUDeviceLostReason reason,
         WGPUStringView message, void *userdata) {
        std::cout << "Device lost: reason " << reason;
        if (message.data)
          std::cout << " (" << message.data << ")";
        std::cout << std::endl;
      };
  descriptor.uncapturedErrorCallbackInfo2.callback =
      [](const WGPUDevice *device, WGPUErrorType type, WGPUStringView message,
         void *userdata1, void *userdata2) {
        switch (type) {
        case WGPUErrorType_NoError:
          break;
        case WGPUErrorType_Validation:
          std::cout << "Validation error: " << message.data << std::endl;
          break;
        case WGPUErrorType_OutOfMemory:
          std::cout << "Out of Memory: " << message.data << std::endl;
          break;
        case WGPUErrorType_Internal:
          std::cout << "Internal error: " << message.data << std::endl;
          break;
        case WGPUErrorType_Unknown:
          std::cout << "Unknown error: " << message.data << std::endl;
          break;
        case WGPUErrorType_DeviceLost:
          std::cout << "Device lost: " << message.data << std::endl;
          break;
        case WGPUErrorType_Force32:
          std::cout << "Force32 error: " << message.data << std::endl;
          break;
        default:
          break;
        }
      };

  wgpuAdapterRequestDevice(adapter, &descriptor, onDeviceRequestEnded,
                           &userData);
#endif
#ifdef __EMSCRIPTEN__
  while (!userData.requestEnded) {
    emscripten_sleep(100);
  }
#endif // __EMSCRIPTEN__
  device = userData.device;
}
void WebGPURenderer::PrepareQueue() { queue = wgpuDeviceGetQueue(device); }
void WebGPURenderer::ConfigSurface(uint32_t width, uint32_t height) {
  WGPUSurfaceConfiguration config = {};
  config.nextInChain = nullptr;

  config.width = width;
  config.height = height;
  config.usage = WGPUTextureUsage_RenderAttachment;
  surfaceFormat = WGPUTextureFormat_RGBA8Unorm;
  config.format = surfaceFormat;
  config.viewFormatCount = 0;
  config.viewFormats = nullptr;
  config.device = device;
  config.presentMode = WGPUPresentMode_Fifo;
  config.alphaMode = WGPUCompositeAlphaMode_Auto;
  wgpuSurfaceConfigure(surface, &config);
}
WebGPURenderer::Texture WebGPURenderer::CreateTexture(const void *data,
                                                      size_t size, int width,
                                                      int height) {
  Texture texture{};
  WGPUTextureFormat format = WGPUTextureFormat_RGBA8Unorm;
  WGPUTextureDescriptor descriptor{
      .nextInChain = nullptr,
      .label = GetStringView("Paranoixa Texture"),
      .usage = WGPUTextureUsage_CopyDst | WGPUTextureUsage_TextureBinding,
      .dimension = WGPUTextureDimension_2D,
      .size = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1},
      .format = WGPUTextureFormat_RGBA8Unorm,
      .mipLevelCount = 1,
      .sampleCount = 1,
      .viewFormatCount = 1,
      .viewFormats = &format};

  texture.texture = wgpuDeviceCreateTexture(device, &descriptor);

  WGPUImageCopyTexture imageCopyTexture{
      .texture = texture.texture,
      .mipLevel = 0,
      .origin = {0, 0, 0},
      .aspect = WGPUTextureAspect_All,
  };

  WGPUTextureDataLayout layout{
      .offset = 0,
      .bytesPerRow = 4 * static_cast<uint32_t>(width),
      .rowsPerImage = static_cast<uint32_t>(height),
  };

  WGPUExtent3D extent{.width = static_cast<uint32_t>(width),
                      .height = static_cast<uint32_t>(height),
                      .depthOrArrayLayers = 1};

  wgpuQueueWriteTexture(queue, &imageCopyTexture, data, size, &layout, &extent);

  WGPUTextureViewDescriptor viewDescriptor{
      .nextInChain = nullptr,
      .label = GetStringView("Paranoixa Texture view"),
      .format = WGPUTextureFormat_RGBA8Unorm,
      .dimension = WGPUTextureViewDimension_2D,
      .baseMipLevel = 0,
      .mipLevelCount = 1,
      .baseArrayLayer = 0,
      .arrayLayerCount = 1,
      .aspect = WGPUTextureAspect_All,
  };
  texture.view = wgpuTextureCreateView(texture.texture, &viewDescriptor);

  return texture;
}
WGPUBuffer WebGPURenderer::CreateBuffer(uint64_t size, WGPUBufferUsage usage) {
  WGPUBufferDescriptor bufferDesc{};
  bufferDesc.nextInChain = nullptr;
  bufferDesc.label = GetStringView("Paranoixa Buffer");
  bufferDesc.size = size;
  bufferDesc.usage = usage;
  bufferDesc.mappedAtCreation = true;
  return wgpuDeviceCreateBuffer(device, &bufferDesc);
}
void WebGPURenderer::PrepareSampler() {
  WGPUSamplerDescriptor samplerDesc{};
  samplerDesc.nextInChain = nullptr;
  samplerDesc.label = GetStringView("Paranoixa Sampler");
  samplerDesc.minFilter = WGPUFilterMode_Nearest;
  samplerDesc.magFilter = WGPUFilterMode_Nearest;
  samplerDesc.addressModeU = WGPUAddressMode_Repeat;
  samplerDesc.addressModeV = WGPUAddressMode_Repeat;
  samplerDesc.addressModeW = WGPUAddressMode_Repeat;
  samplerDesc.lodMinClamp = 0.0f;
  samplerDesc.lodMaxClamp = 0.0f;
  samplerDesc.compare = WGPUCompareFunction_Undefined;
  samplerDesc.maxAnisotropy = 1;
  samplerDesc.mipmapFilter = WGPUMipmapFilterMode_Nearest;
  sampler = wgpuDeviceCreateSampler(device, &samplerDesc);
}
WGPUTextureView WebGPURenderer::GetNextSurfaceTextureView() {

  WGPUSurfaceTexture surfaceTexture;
  wgpuSurfaceGetCurrentTexture(surface, &surfaceTexture);
  if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_Success) {
    return nullptr;
  }

  WGPUTextureViewDescriptor viewDescriptor;
  viewDescriptor.nextInChain = nullptr;
  viewDescriptor.label = GetStringView("Paranoixa Surface texture view");
  viewDescriptor.format = wgpuTextureGetFormat(surfaceTexture.texture);
  viewDescriptor.dimension = WGPUTextureViewDimension_2D;
  viewDescriptor.baseMipLevel = 0;
  viewDescriptor.mipLevelCount = 1;
  viewDescriptor.baseArrayLayer = 0;
  viewDescriptor.arrayLayerCount = 1;
  viewDescriptor.aspect = WGPUTextureAspect_All;
#ifdef DAWN
  viewDescriptor.usage = WGPUTextureUsage_RenderAttachment;
#endif
  WGPUTextureView targetView =
      wgpuTextureCreateView(surfaceTexture.texture, &viewDescriptor);
  return targetView;
}
#ifdef DAWN
WGPUStringView WebGPURenderer::GetStringView(const char *str) {
  return {strWGPU_STRLEN};
}
#else
const char *WebGPURenderer::GetStringView(const char *str) { return {str}; }
#endif
void WebGPURenderer::InitializePipeline() {
  WGPUShaderModuleDescriptor shaderDesc{};
  shaderDesc.label = GetStringView("Paranoixa Shader module");
  WGPUShaderModuleWGSLDescriptor wgslDesc{};
  wgslDesc.chain.next = nullptr;
#ifdef DAWN
  wgslDesc.chain.sType = WGPUSType_ShaderSourceWGSL;
#endif
  shaderDesc.nextInChain = &wgslDesc.chain;

  std::vector<char> vertCode, fragCode;
  // load in plain

#ifdef DAWN
  wgslDesc.code = {vertCode.data(), vertCode.size()};
  WGPUShaderModule vertShaderModule =
      wgpuDeviceCreateShaderModule(device, &shaderDesc);
  wgslDesc.code = {fragCode.data(), fragCode.size()};
  WGPUShaderModule fragShaderModule =
      wgpuDeviceCreateShaderModule(device, &shaderDesc);
#else
  wgslDesc.code = {vertCode.data()};
  WGPUShaderModule vertShaderModule =
      wgpuDeviceCreateShaderModule(device, &shaderDesc);
  wgslDesc.code = {fragCode.data()};
  WGPUShaderModule fragShaderModule =
      wgpuDeviceCreateShaderModule(device, &shaderDesc);
#endif

  WGPURenderPipelineDescriptor pipelineDesc{};
  pipelineDesc.nextInChain = nullptr;
  pipelineDesc.label = GetStringView("Paranoixa Pipeline");
  pipelineDesc.vertex.bufferCount = 1;
  WGPUVertexBufferLayout vertexBufferLayout{};
  vertexBufferLayout.arrayStride = sizeof(float) * 8;
  vertexBufferLayout.stepMode = WGPUVertexStepMode_Vertex;
  vertexBufferLayout.attributeCount = 3;
  WGPUVertexAttribute attributes[3] = {
      {.format = WGPUVertexFormat_Float32x3, .offset = 0, .shaderLocation = 0},
      {.format = WGPUVertexFormat_Float32x2,
       .offset = sizeof(float) * 3,
       .shaderLocation = 1},
      {.format = WGPUVertexFormat_Float32x3,
       .offset = sizeof(float) * 5,
       .shaderLocation = 2},
  };
  vertexBufferLayout.attributes = attributes;
  pipelineDesc.vertex.buffers = &vertexBufferLayout;

  pipelineDesc.vertex.module = vertShaderModule;
  pipelineDesc.vertex.entryPoint = GetStringView("main");
  pipelineDesc.vertex.constantCount = 0;
  pipelineDesc.vertex.constants = nullptr;

  pipelineDesc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
  pipelineDesc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
  pipelineDesc.primitive.frontFace = WGPUFrontFace_CCW;
  pipelineDesc.primitive.cullMode = WGPUCullMode_None;

  WGPUFragmentState fragmentState{};
  fragmentState.module = fragShaderModule;
  fragmentState.entryPoint = GetStringView("main");
  fragmentState.constantCount = 0;
  fragmentState.constants = nullptr;

  WGPUBlendState blendState{};
  blendState.color.srcFactor = WGPUBlendFactor_SrcAlpha;
  blendState.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
  blendState.color.operation = WGPUBlendOperation_Add;
  blendState.alpha.srcFactor = WGPUBlendFactor_Zero;
  blendState.alpha.dstFactor = WGPUBlendFactor_One;
  blendState.alpha.operation = WGPUBlendOperation_Add;

  WGPUColorTargetState colorTarget{};
  colorTarget.format = surfaceFormat;
  colorTarget.blend = &blendState;
  colorTarget.writeMask = WGPUColorWriteMask_All; // We could write to only some
                                                  // of the color channels.

  // We have only one target because our render pass has only one output color
  // attachment.
  fragmentState.targetCount = 1;
  fragmentState.targets = &colorTarget;
  pipelineDesc.fragment = &fragmentState;

  // We do not use stencil/depth testing for now
  pipelineDesc.depthStencil = nullptr;

  // Samples per pixel
  pipelineDesc.multisample.count = 1;

  // Default value for the mask, meaning "all bits on"
  pipelineDesc.multisample.mask = ~0u;

  // Default value as well (irrelevant for count = 1 anyways)
  pipelineDesc.multisample.alphaToCoverageEnabled = false;

  constexpr WGPUBindGroupLayoutEntry entry[] = {
      {
          .binding = 0,
          .visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment,
          .buffer = {},
          .sampler = {},
          .texture =
              {
                  .nextInChain = nullptr,
                  .sampleType = WGPUTextureSampleType_Float,
                  .viewDimension = WGPUTextureViewDimension_2D,
                  .multisampled = false,
              },
      },
      {
          .binding = 1,
          .visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment,
          .buffer = {},
          .sampler =
              {
                  .nextInChain = nullptr,
                  .type = WGPUSamplerBindingType_NonFiltering,
              },
          .texture = {},
      },
  };
  WGPUBindGroupLayoutDescriptor bindGroupLayoutDesc{
      .nextInChain = nullptr,
      .label = GetStringView("Paranoixa Bind group layout"),
      .entryCount = COUNT_OF(entry),
      .entries = entry,
  };
  auto bindGroupLayout =
      wgpuDeviceCreateBindGroupLayout(device, &bindGroupLayoutDesc);

  WGPUBindGroupEntry bindGroupEntry[] = {
      {.nextInChain = nullptr,
       .binding = 0,
       .buffer = nullptr,
       .offset = 0,
       .size = 0,
       .sampler = nullptr,
       .textureView = texture.view},
      {.nextInChain = nullptr,
       .binding = 1,
       .buffer = nullptr,
       .offset = 0,
       .size = 0,
       .sampler = sampler,
       .textureView = nullptr},
  }; // namespace paranoixa
  WGPUBindGroupDescriptor bindGroupDesc{
      .nextInChain = nullptr,
      .label = GetStringView("Paranoixa Texture bind group"),
      .layout = bindGroupLayout,
      .entryCount = COUNT_OF(bindGroupEntry),
      .entries = bindGroupEntry};

  bindGroup = wgpuDeviceCreateBindGroup(device, &bindGroupDesc);

  WGPUPipelineLayoutDescriptor pipelineLayoutDesc{};
  pipelineLayoutDesc.nextInChain = nullptr;
  pipelineLayoutDesc.label = GetStringView("Paranoixa Pipeline layout");
  pipelineLayoutDesc.bindGroupLayoutCount = 1;
  pipelineLayoutDesc.bindGroupLayouts = &bindGroupLayout;
  auto pipelineLayout =
      wgpuDeviceCreatePipelineLayout(device, &pipelineLayoutDesc);

  pipelineDesc.layout = pipelineLayout;

  pipeline = wgpuDeviceCreateRenderPipeline(device, &pipelineDesc);

  // We no longer need to access the shader module
  wgpuShaderModuleRelease(vertShaderModule);
} // namespace paranoixa
} // namespace paranoixa
#endif // __ANDROID__
#endif
