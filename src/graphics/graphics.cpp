#include "graphics.hpp"
#include <cassert>
#include <core/allocator/global_allocator.hpp>
#include <core/logger/log.hpp>
#include <core/profiler.hpp>
#include <gpu/builtin_shader.hpp>
#include <gpu/gpu.hpp>
#include <graphics/builtin_pipeline.hpp>
#include <graphics/texture/render_texture.hpp>
#include <gui/gui.hpp>
#include <math/transform/transform.hpp>
#include <platform/window/window.hpp>
#include <script/script.hpp>

#include <SDL3/SDL.h>

#include <algorithm>
#include <vector>

namespace sinen {
// Variables
static Color clearColor = Palette::black();
enum class GraphicsPass {
  TwoD,
  ThreeD,
};
static GraphicsPass currentGraphicsPass = GraphicsPass::TwoD;
static std::optional<Camera2D> currentCamera2D;
static std::optional<Camera3D> currentCamera3D;
static std::list<std::function<void()>> preDrawFuncs;
static std::list<std::function<void()>> postDrawFuncs;
static Ptr<gpu::Backend> backend;
static Ptr<gpu::Device> device;
static Ptr<gpu::Texture> depthTexture;
static Ptr<gpu::Sampler> sampler;
static Ptr<gpu::Sampler> fontSampler;
static std::optional<GraphicsPipeline> currentPipeline;
static std::optional<GraphicsPipeline> customPipeline;
static Ptr<gpu::CommandBuffer> mainCommandBuffer;
static Ptr<gpu::CommandBuffer> currentCommandBuffer;
static Ptr<gpu::RenderPass> currentRenderPass;
static bool isFrameStarted = true;
static bool isPrevDepthEnabled = true;
static bool isChangedRenderTarget = false;
static uint32_t drawCallCountPerFrame = 0;
static Array<gpu::ColorTargetInfo> colorTargets = Array<gpu::ColorTargetInfo>();
static gpu::DepthStencilTargetInfo depthStencilInfo;
static Array<gpu::ColorTargetInfo> currentColorTargets;
static gpu::DepthStencilTargetInfo currentDepthStencilInfo;
static Hashmap<UInt32, Ptr<Texture>> currentTextureBindings;
static Hashmap<UInt32, Ptr<gpu::AccelerationStructure>>
    currentAccelerationStructureBindings;
static Model box = Model();
static Model sprite = Model();

// Functions
static void prepareRenderPassFrame();
static void setupShapes();
static void beginRenderPass(bool depthEnabled, gpu::LoadOp loadOp);
static Vec2 validRenderSize();
static Ptr<gpu::Texture> createDepthTexture(const Vec2 &size);
static Vec2 renderTargetSize(const Ptr<gpu::Texture> &texture);

static GPUBackendAPI selectBackendAPI() {
#ifdef SINEN_PLATFORM_EMSCRIPTEN
  return GPUBackendAPI::WebGPU;
#else
  const char *backendName = SDL_getenv("SINEN_GPU_BACKEND");
  if (backendName == nullptr || backendName[0] == '\0') {
#ifdef SINEN_PLATFORM_WINDOWS
    return GPUBackendAPI::D3D12;
#else
    return GPUBackendAPI::SDLGPU;
#endif
  }

#ifdef SINEN_PLATFORM_WINDOWS
  if (std::strcmp(backendName, "d3d12") == 0 ||
      std::strcmp(backendName, "direct3d12") == 0) {
    return GPUBackendAPI::D3D12;
  }
#endif
  if (std::strcmp(backendName, "vulkan") == 0) {
    return GPUBackendAPI::Vulkan;
  }
#ifdef SINEN_ENABLE_WEBGPU
  if (std::strcmp(backendName, "webgpu") == 0 ||
      std::strcmp(backendName, "wgpu") == 0 ||
      std::strcmp(backendName, "dawn") == 0) {
    return GPUBackendAPI::WebGPU;
  }
#endif
  return GPUBackendAPI::SDLGPU;
#endif
}

static void setFullWindowViewport(const Ptr<gpu::RenderPass> &renderPass) {
  Rect rect;
  // SDL_Rect safeArea;
  // SDL_GetWindowSafeArea(WindowSystem::get_sdl_window(), &safeArea);
  rect.x = 0;
  rect.y = 0;
  const Vec2 targetSize = renderTargetSize(currentColorTargets[0].texture);
  rect.width = targetSize.x;
  rect.height = targetSize.y;

  gpu::Viewport viewport{};
  viewport.x = rect.x;
  viewport.y = rect.y;
  viewport.width = rect.width;
  viewport.height = rect.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  renderPass->setViewport(viewport);
  renderPass->setScissor(rect.x, rect.y, rect.width, rect.height);
}
bool Graphics::initialize() {
  backend = gpu::RHI::createBackend(GlobalAllocator::get(), selectBackendAPI());
  if (!backend)
    return false;
  gpu::Device::CreateInfo info{};
  info.allocator = GlobalAllocator::get();
  info.debugMode = true;
  device = backend->createDevice(info);
  if (!device) {
    Log::error("Failed to create GPU device");
    return false;
  }

  auto *window = Window::getSdlWindow();
  device->claimWindow(window);
  BuiltinShader::initialize();
  BuiltinPipeline::initialize();

  // Create depth stencil target
  {
    depthTexture = createDepthTexture(validRenderSize());
    if (!depthTexture) {
      Log::error("Failed to create depth texture");
      return false;
    }
  }

  // Default sampler
  gpu::Sampler::CreateInfo samplerInfo{};
  samplerInfo.allocator = GlobalAllocator::get();
  samplerInfo.minFilter = gpu::Filter::Linear;
  samplerInfo.magFilter = gpu::Filter::Linear;
  samplerInfo.mipmapMode = gpu::MipmapMode::Linear;
  samplerInfo.addressModeU = gpu::AddressMode::Repeat;
  samplerInfo.addressModeV = gpu::AddressMode::Repeat;
  samplerInfo.addressModeW = gpu::AddressMode::Repeat;
  samplerInfo.minLod = 0.0f;
  samplerInfo.maxLod = 1000.0f;
  samplerInfo.maxAnisotropy = 1.f;
  sampler = device->createSampler(samplerInfo);
  if (!sampler) {
    Log::error("Failed to create default sampler");
    return false;
  }

  gpu::Sampler::CreateInfo fontSamplerInfo = samplerInfo;
  fontSamplerInfo.minFilter = gpu::Filter::Linear;
  fontSamplerInfo.magFilter = gpu::Filter::Linear;
  fontSamplerInfo.mipmapMode = gpu::MipmapMode::Nearest;
  fontSamplerInfo.addressModeU = gpu::AddressMode::ClampToEdge;
  fontSamplerInfo.addressModeV = gpu::AddressMode::ClampToEdge;
  fontSamplerInfo.addressModeW = gpu::AddressMode::ClampToEdge;
  fontSampler = device->createSampler(fontSamplerInfo);
  if (!fontSampler) {
    Log::error("Failed to create font sampler");
    return false;
  }

  colorTargets.push_back({});
  depthStencilInfo.texture = depthTexture;
  depthStencilInfo.loadOp = gpu::LoadOp::Clear;
  depthStencilInfo.storeOp = gpu::StoreOp::Store;
  depthStencilInfo.clearDepth = 1.0f;
  depthStencilInfo.clearStencil = 0;
  depthStencilInfo.cycle = false;
  depthStencilInfo.stencilLoadOp = gpu::LoadOp::Clear;
  depthStencilInfo.stencilStoreOp = gpu::StoreOp::Store;
  setupShapes();

  return true;
}

void Graphics::shutdown() {
  device.reset();
  backend.reset();
}

void Graphics::render() {
  ZoneScopedN("Graphics::render");
  {
    ZoneScopedN("preDraw");
    auto funcs = std::move(preDrawFuncs);
    preDrawFuncs.clear();
    for (auto &f : funcs) {
      f();
    }
  }

  currentRenderPass = nullptr;
  currentCommandBuffer.reset();
  mainCommandBuffer.reset();
  if (!colorTargets.empty()) {
    colorTargets[0].texture = nullptr;
  }
  currentColorTargets.clear();

  auto commandBuffer = device->acquireCommandBuffer({GlobalAllocator::get()});
  if (commandBuffer == nullptr) {
    return;
  }
  auto swapchainTexture = device->acquireSwapchainTexture(commandBuffer);
  if (swapchainTexture == nullptr) {
    return;
  }
  mainCommandBuffer = commandBuffer;
  currentCommandBuffer = commandBuffer;
  colorTargets[0].texture = swapchainTexture;
  currentColorTargets = colorTargets;
  const Vec2 targetSize = renderTargetSize(swapchainTexture);
  const Vec2 depthSize = renderTargetSize(depthTexture);
  if (Window::resized() || depthSize.x != targetSize.x ||
      depthSize.y != targetSize.y) {
    depthTexture = createDepthTexture(targetSize);
    depthStencilInfo.texture = depthTexture;
  }
  currentDepthStencilInfo = depthStencilInfo;

  isFrameStarted = true;
  currentGraphicsPass = GraphicsPass::TwoD;
  currentCamera2D = std::nullopt;
  currentCamera3D = std::nullopt;
  Gui::newFrame();
  drawCallCountPerFrame = 0;
  {
    ZoneScopedN("Script::drawScene");
    Script::drawScene();
  }
  for (auto &f : postDrawFuncs) {
    f();
  }

  // Rendering

  if (drawCallCountPerFrame == 0) {
    // Clear screen
    beginRenderPass(true, gpu::LoadOp::Clear);
  }
  commandBuffer->endRenderPass(currentRenderPass);

  {
    ZoneScopedN("GPU submit");
    device->submitCommandBuffer(commandBuffer);
  }
  {
    ZoneScopedN("GPU wait idle");
    device->waitForGpuIdle();
  }
}
struct Transform2D {
  Vec2 position;
  float rotation;
  Vec2 scale;
};
struct FontFragmentParams {
  Color textColor;
  Vec4 atlasParams;
};

static bool isRgba8Format(gpu::TextureFormat format) {
  return format == gpu::TextureFormat::R8G8B8A8_UNORM ||
         format == gpu::TextureFormat::B8G8R8A8_UNORM;
}

static UInt32 alignTo(UInt32 value, UInt32 alignment) {
  return (value + alignment - 1) & ~(alignment - 1);
}

static void copyTextureBytesWithFormatConversion(const UInt8 *src, UInt8 *dst,
                                                 UInt32 width, UInt32 height,
                                                 UInt32 srcBytesPerRow,
                                                 UInt32 dstBytesPerRow,
                                                 gpu::TextureFormat srcFormat,
                                                 gpu::TextureFormat dstFormat) {
  const bool swapRB = isRgba8Format(srcFormat) && isRgba8Format(dstFormat) &&
                      srcFormat != dstFormat;
  if (!swapRB) {
    for (UInt32 y = 0; y < height; ++y) {
      memcpy(dst + static_cast<size_t>(dstBytesPerRow) * y,
             src + static_cast<size_t>(srcBytesPerRow) * y,
             static_cast<size_t>(width) * 4);
    }
    return;
  }

  for (UInt32 y = 0; y < height; ++y) {
    const UInt8 *srcRow = src + static_cast<size_t>(srcBytesPerRow) * y;
    UInt8 *dstRow = dst + static_cast<size_t>(dstBytesPerRow) * y;
    for (UInt32 x = 0; x < width; ++x) {
      const size_t offset = static_cast<size_t>(x) * 4;
      dstRow[offset + 0] = srcRow[offset + 2];
      dstRow[offset + 1] = srcRow[offset + 1];
      dstRow[offset + 2] = srcRow[offset + 0];
      dstRow[offset + 3] = srcRow[offset + 3];
    }
  }
}

static void
bindCurrentTextureSamplers(const Ptr<gpu::RenderPass> &renderPass,
                           const Ptr<gpu::Sampler> &textureSampler = sampler) {
  if (currentTextureBindings.empty()) {
    return;
  }

#ifdef SINEN_PLATFORM_WINDOWS
  if (device && device->getBackendAPI() != GPUBackendAPI::D3D12) {
#endif
    for (auto &binding : currentTextureBindings) {
      if (!binding.second) {
        continue;
      }
      renderPass->bindFragmentSampler(
          binding.first, {textureSampler, binding.second->getRaw()});
    }
    return;
#ifdef SINEN_PLATFORM_WINDOWS
  }
#endif

  UInt32 maxSlot = 0;
  for (const auto &binding : currentTextureBindings) {
    if (binding.first > maxSlot) {
      maxSlot = binding.first;
    }
  }

  Array<gpu::TextureSamplerBinding> textureSamplers;
  textureSamplers.resize(maxSlot + 1);
  for (const auto &binding : currentTextureBindings) {
    if (!binding.second) {
      continue;
    }
    textureSamplers[binding.first] = {textureSampler, binding.second->getRaw()};
  }
  renderPass->bindFragmentSamplers(0, textureSamplers);
}

static void
bindCurrentAccelerationStructures(const Ptr<gpu::RenderPass> &renderPass) {
  if (currentAccelerationStructureBindings.empty()) {
    return;
  }
  UInt32 maxSlot = 0;
  for (const auto &binding : currentAccelerationStructureBindings) {
    maxSlot = std::max(maxSlot, binding.first);
  }
  Array<Ptr<gpu::AccelerationStructure>> accelerationStructures;
  accelerationStructures.resize(maxSlot + 1);
  for (const auto &binding : currentAccelerationStructureBindings) {
    accelerationStructures[binding.first] = binding.second;
  }
  renderPass->bindAccelerationStructures(0, accelerationStructures);
}

static void drawBase2D(const Array<Transform2D> &transforms, const Model &model,
                       const Ptr<gpu::Sampler> &textureSampler = sampler) {
  assert(currentPipeline.has_value());
  auto vertexBufferBindings = Array<gpu::BufferBinding>{};
  auto indexBufferBinding = gpu::BufferBinding{};
  auto textureSamplers = Array<gpu::TextureSamplerBinding>{};
  const Camera2D windowCamera(Window::size());
  const Camera2D *camera2D =
      currentCamera2D.has_value() ? &currentCamera2D.value() : &windowCamera;
  auto ratio = camera2D->windowRatio();
  Mat4 wvp;
  Array<Mat4> instanceData;
  const auto cameraSize = camera2D->size();
  const Vec2 cameraHalf = cameraSize * 0.5f;
  const auto pos = Vec2(transforms[0].position.x - cameraHalf.x,
                        cameraHalf.y - transforms[0].position.y) *
                   ratio;
  auto scale = transforms[0].scale * 0.5f * ratio;
  {
    Transform transform;
    transform.setPosition(Vec3(pos.x, pos.y, 0.0f));
    transform.setRotation(Vec3(0, 0, transforms[0].rotation));
    transform.setScale(Vec3(scale.x, scale.y, 1.0f));

    wvp = transform.getWorldMatrix();
  }
  auto viewproj = Mat4(1.0f);

  viewproj[0][0] = 2.f / Window::size().x;
  viewproj[1][1] = 2.f / Window::size().y;
  wvp = viewproj * wvp;
  if (transforms.size() > 1) {
    for (auto &i : transforms) {
      Transform transform;
      const auto instancePos =
          Vec2(i.position.x - cameraHalf.x, cameraHalf.y - i.position.y) *
          ratio;
      transform.setPosition(Vec3(instancePos.x, instancePos.y, 0.0f));
      transform.setRotation(Vec3(0, 0, i.rotation));
      transform.setScale(
          Vec3(i.scale.x * 0.5f * ratio.x, i.scale.y * 0.5f * ratio.y, 1.0f));
      instanceData.push_back(transform.getWorldMatrix());
    }
  }
  drawCallCountPerFrame++;
  prepareRenderPassFrame();
  SDL_assert(currentRenderPass);

  assert(model.vertexBuffer != nullptr);
  assert(model.indexBuffer != nullptr);

  vertexBufferBindings.emplace_back(
      gpu::BufferBinding{.buffer = model.vertexBuffer, .offset = 0});
  indexBufferBinding =
      gpu::BufferBinding{.buffer = model.indexBuffer, .offset = 0};

  auto commandBuffer = currentCommandBuffer;
  auto renderPass = currentRenderPass;
  renderPass->bindGraphicsPipeline(currentPipeline.value().get());
  bindCurrentTextureSamplers(renderPass, textureSampler);
  bindCurrentAccelerationStructures(renderPass);
  renderPass->bindVertexBuffers(0, vertexBufferBindings);
  renderPass->bindIndexBuffer(indexBufferBinding,
                              gpu::IndexElementSize::Uint32);

  commandBuffer->pushVertexUniformData(0, &wvp, sizeof(wvp));
  renderPass->drawIndexedPrimitives(model.getMesh().data()->indices.size(), 1,
                                    0, 0, 0);
  currentPipeline = std::nullopt;
  currentTextureBindings.clear();
  currentAccelerationStructureBindings.clear();
}

static Mat4 cubemapViewProjection(const Camera3D &camera) {
  Mat4 view = camera.getView();
  view[0][3] = 0.0f;
  view[1][3] = 0.0f;
  view[2][3] = 0.0f;
  return camera.getProjection() * view;
}

static void drawBaseCubemap(const Model &model) {
  assert(currentPipeline.has_value());
  SDL_assert(currentGraphicsPass == GraphicsPass::ThreeD);
  SDL_assert(currentCamera3D.has_value());
  if (currentGraphicsPass != GraphicsPass::ThreeD ||
      !currentCamera3D.has_value()) {
    return;
  }

  const Mat4 wvp = cubemapViewProjection(currentCamera3D.value());
  drawCallCountPerFrame++;
  prepareRenderPassFrame();

  assert(model.vertexBuffer != nullptr);
  assert(model.indexBuffer != nullptr);

  Array<gpu::BufferBinding> vertexBufferBindings;
  vertexBufferBindings.emplace_back(
      gpu::BufferBinding{.buffer = model.vertexBuffer, .offset = 0});
  const gpu::BufferBinding indexBufferBinding{
      .buffer = model.indexBuffer, .offset = 0};

  auto commandBuffer = currentCommandBuffer;
  auto renderPass = currentRenderPass;
  renderPass->bindGraphicsPipeline(currentPipeline.value().get());
  bindCurrentTextureSamplers(renderPass);
  bindCurrentAccelerationStructures(renderPass);
  renderPass->bindVertexBuffers(0, vertexBufferBindings);
  renderPass->bindIndexBuffer(indexBufferBinding,
                              gpu::IndexElementSize::Uint32);

  commandBuffer->pushVertexUniformData(0, &wvp, sizeof(wvp));
  renderPass->drawIndexedPrimitives(
      static_cast<uint32_t>(model.getMesh().data()->indices.size()), 1, 0, 0,
      0);
  currentPipeline = std::nullopt;
  currentTextureBindings.clear();
  currentAccelerationStructureBindings.clear();
}

static void drawBase3D(const Array<Transform> transforms, const Model &model) {
  ZoneScopedN("drawBase3D");
  assert(currentPipeline.has_value());
  SDL_assert(currentGraphicsPass == GraphicsPass::ThreeD);
  SDL_assert(currentCamera3D.has_value());
  if (currentGraphicsPass != GraphicsPass::ThreeD ||
      !currentCamera3D.has_value()) {
    return;
  }
  const Camera3D &camera = currentCamera3D.value();
  auto vertexBufferBindings = Array<gpu::BufferBinding>();
  auto indexBufferBinding = gpu::BufferBinding{};
  Mat4 wvp;
  Mat4 viewproj;
  Array<Mat4> instanceData;
  {
    viewproj = camera.getProjection() * camera.getView();
    wvp = viewproj * transforms[0].getWorldMatrix();
  }
  if (transforms.size() > 1) {
    for (auto &i : transforms) {
      instanceData.push_back(i.getWorldMatrix());
    }
  }
  drawCallCountPerFrame++;
  prepareRenderPassFrame();

  auto instanceSize = sizeof(Mat4) * instanceData.size();
  bool isInstance = instanceSize > 0;
  Ptr<gpu::Buffer> instanceBuffer = nullptr;
  if (isInstance) {
    if (!currentPipeline.has_value())
      currentPipeline = BuiltinPipeline::getInstanced3D();
    gpu::Buffer::CreateInfo instanceBufferInfo{};
    instanceBufferInfo.allocator = GlobalAllocator::get();
    instanceBufferInfo.size = instanceSize;
    instanceBufferInfo.usage = gpu::BufferUsage::Vertex;
    instanceBuffer = device->createBuffer(instanceBufferInfo);
    Ptr<gpu::TransferBuffer> transferBuffer;
    {
      gpu::TransferBuffer::CreateInfo info{};
      info.allocator = GlobalAllocator::get();
      info.size = instanceSize;
      info.usage = gpu::TransferBufferUsage::Upload;
      transferBuffer = device->createTransferBuffer(info);
      auto *pMapped = transferBuffer->map(false);
      if (pMapped) {
        memcpy(pMapped, instanceData.data(), instanceSize);
      }
      transferBuffer->unmap();
    }
    {
      gpu::CommandBuffer::CreateInfo info{};
      info.allocator = GlobalAllocator::get();
      auto commandBuffer = device->acquireCommandBuffer(info);
      {
        auto copyPass = commandBuffer->beginCopyPass();
        {
          gpu::BufferTransferInfo src{};
          src.offset = 0;
          src.transferBuffer = transferBuffer;
          gpu::BufferRegion dst{};
          dst.offset = 0;
          dst.size = instanceSize;
          dst.buffer = instanceBuffer;
          copyPass->uploadBuffer(src, dst, false);
        }
        commandBuffer->endCopyPass(copyPass);
      }
      device->submitCommandBuffer(commandBuffer);
    }
  }

  vertexBufferBindings.emplace_back(
      gpu::BufferBinding{.buffer = model.vertexBuffer, .offset = 0});
  UInt32 lod = 0;
  if (transforms.size() == 1) {
    lod = model.selectLod(
        (transforms[0].getPosition() - camera.getPosition()).length());
  }
  Ptr<gpu::Buffer> selectedIndexBuffer = model.indexBuffer;
  if (lod < model.lodIndexBuffers.size() && model.lodIndexBuffers[lod]) {
    selectedIndexBuffer = model.lodIndexBuffers[lod];
  }
  indexBufferBinding =
      gpu::BufferBinding{.buffer = selectedIndexBuffer, .offset = 0};
  if (isInstance) {
    vertexBufferBindings.emplace_back(
        gpu::BufferBinding{.buffer = instanceBuffer, .offset = 0});
  }
  const auto &pipelineFeatures = currentPipeline.value().getFeatureFlags();
  if (pipelineFeatures.test(GraphicsPipeline::FeatureFlag::Animation)) {
    auto animationVertexBuffer = model.animationVertexBuffer;
    SDL_assert(animationVertexBuffer != nullptr);
    vertexBufferBindings.emplace_back(
        gpu::BufferBinding{.buffer = animationVertexBuffer, .offset = 0});
  }
  if (pipelineFeatures.test(GraphicsPipeline::FeatureFlag::Tangent)) {
    auto tangentBuffer = model.tangentBuffer;
    SDL_assert(tangentBuffer != nullptr);
    vertexBufferBindings.emplace_back(
        gpu::BufferBinding{.buffer = tangentBuffer, .offset = 0});
  }
  auto commandBuffer = currentCommandBuffer;
  auto renderPass = currentRenderPass;
  renderPass->bindGraphicsPipeline(currentPipeline.value().get());
  bindCurrentTextureSamplers(renderPass);
  bindCurrentAccelerationStructures(renderPass);
  renderPass->bindVertexBuffers(0, vertexBufferBindings);
  renderPass->bindIndexBuffer(indexBufferBinding,
                              gpu::IndexElementSize::Uint32);

  const Mat4 &vertexParams = isInstance ? viewproj : wvp;
  commandBuffer->pushVertexUniformData(0, &vertexParams, sizeof(vertexParams));
  uint32_t numIndices =
      static_cast<uint32_t>(model.getIndicesForLod(lod).size());
  uint32_t numInstance =
      isInstance ? static_cast<uint32_t>(instanceData.size()) : 1;
  renderPass->drawIndexedPrimitives(numIndices, numInstance, 0, 0, 0);
  currentPipeline = std::nullopt;
  currentTextureBindings.clear();
  currentAccelerationStructureBindings.clear();
}
void Graphics::drawRect(const Rect &rect, const Color &color, float angle) {
  if (customPipeline.has_value() && customPipeline.value().get() != nullptr)
    currentPipeline = customPipeline.value();
  else
    currentPipeline = BuiltinPipeline::getRect2D();
  Array<Transform2D> transforms(1, {rect.center(), angle, rect.size()});
  currentCommandBuffer->pushFragmentUniformData(1, &color, sizeof(Color));
  drawBase2D(transforms, sprite);
}
void Graphics::drawImage(const Ptr<Texture> &texture, const Rect &rect,
                         float angle) {
  if (customPipeline.has_value() && customPipeline.value().get() != nullptr)
    currentPipeline = customPipeline.value();
  else
    currentPipeline = BuiltinPipeline::getDefault2D();
  Array<Transform2D> transforms(1, {rect.center(), angle, rect.size()});
  setTexture(0, texture);
  drawBase2D(transforms, sprite);
}
void Graphics::drawText(StringView text, const Font &font, const Vec2 &position,
                        const Color &color, float textSize, float angle) {
  if (customPipeline.has_value() && customPipeline.value().get() != nullptr)
    currentPipeline = customPipeline.value();
  else
    currentPipeline = BuiltinPipeline::getFont2D();
  TextDrawData textData = font.makeTextDrawData(text);
  if (!textData.valid || textData.texture == nullptr) {
    return;
  }

  Model model;
  model.loadFromVertexArray(textData.mesh);
  const auto scale = textSize / static_cast<float>(font.size());
  const auto meshScale = scale * 0.5f;
  Vec2 textPosition = position;
  const auto meshData = textData.mesh.data();
  if (meshData != nullptr && !meshData->vertices.empty()) {
    auto minX = meshData->vertices[0].position.x;
    auto maxY = meshData->vertices[0].position.y;
    for (const auto &vertex : meshData->vertices) {
      minX = std::min(minX, vertex.position.x);
      maxY = std::max(maxY, vertex.position.y);
    }
    textPosition.x -= minX * meshScale;
    textPosition.y += maxY * meshScale;
  }
  Array<Transform2D> transforms(1, {textPosition, angle, Vec2(scale)});
  setTexture(0, textData.texture);
  const Vec2 atlasSize = textData.texture->size();
  const FontFragmentParams params{
      color, Vec4(atlasSize.x, atlasSize.y, textData.distanceFieldRange, 0.0f)};
  currentCommandBuffer->pushFragmentUniformData(1, &params, sizeof(params));

  drawBase2D(transforms, model, fontSampler);
}
void Graphics::drawCubemap(const Ptr<Texture> &cubemap) {
  if (!cubemap || !cubemap->getRaw()) {
    return;
  }

  if (customPipeline.has_value() && customPipeline.value().get() != nullptr)
    currentPipeline = customPipeline.value();
  else
    currentPipeline = BuiltinPipeline::getCubemap();

  setTexture(0, cubemap);
  drawBaseCubemap(box);
}
void Graphics::drawModel(const Model &model, const Transform &transform) {
  if (customPipeline.has_value() && customPipeline.value().get() != nullptr)
    currentPipeline = customPipeline.value();
  else
    currentPipeline = BuiltinPipeline::getDefault3D();

  if (model.hasTexture(TextureKey::BaseColor))
    setTexture(0, model.getTexture(TextureKey::BaseColor));
  else {
    auto t = Texture::create();
    t->fill(Palette::white());
    setTexture(0, t);
  }

  drawBase3D(Array<Transform>(1, transform), model);
}
void Graphics::drawModelInstanced(const Model &model,
                                  const Array<Transform> &transforms) {
  if (transforms.empty())
    return;

  if (customPipeline.has_value() && customPipeline.value().get() != nullptr)
    currentPipeline = customPipeline.value();
  else
    currentPipeline = BuiltinPipeline::getInstanced3D();
  if (model.hasTexture(TextureKey::BaseColor))
    setTexture(0, model.getTexture(TextureKey::BaseColor));
  else {
    auto t = Texture::create();
    t->fill(Palette::white());
    setTexture(0, t);
  }
  drawBase3D(transforms, model);
}

static void beginRenderPass(bool depthEnabled, gpu::LoadOp loadOp) {
  colorTargets[0].loadOp = loadOp;
  currentColorTargets = colorTargets;

  if (depthEnabled) {
    currentDepthStencilInfo = depthStencilInfo;
    currentRenderPass = currentCommandBuffer->beginRenderPass(
        currentColorTargets, currentDepthStencilInfo);
  } else {
    currentRenderPass =
        currentCommandBuffer->beginRenderPass(currentColorTargets, {});
  }

  setFullWindowViewport(currentRenderPass);
}

static Vec2 validRenderSize() {
  Vec2 size = Window::size();
  if (auto *window = Window::getSdlWindow()) {
    int width = 0;
    int height = 0;
    SDL_GetWindowSizeInPixels(window, &width, &height);
    if (width > 0 && height > 0) {
      size = Vec2(static_cast<float>(width), static_cast<float>(height));
    }
  }
  if (size.x <= 0.0f || size.y <= 0.0f) {
    size = Vec2(1.0f, 1.0f);
  }
  return size;
}

static Ptr<gpu::Texture> createDepthTexture(const Vec2 &size) {
  gpu::Texture::CreateInfo depthStencilCreateInfo{};
  depthStencilCreateInfo.allocator = GlobalAllocator::get();
  depthStencilCreateInfo.width = static_cast<uint32_t>(std::max(1.0f, size.x));
  depthStencilCreateInfo.height = static_cast<uint32_t>(std::max(1.0f, size.y));
  depthStencilCreateInfo.layerCountOrDepth = 1;
  depthStencilCreateInfo.type = gpu::TextureType::Texture2D;
  depthStencilCreateInfo.usage = gpu::TextureUsage::DepthStencilTarget;
  depthStencilCreateInfo.format = gpu::TextureFormat::D32_FLOAT_S8_UINT;
  depthStencilCreateInfo.numLevels = 1;
  depthStencilCreateInfo.sampleCount = gpu::SampleCount::x1;
  return device->createTexture(depthStencilCreateInfo);
}

static Vec2 renderTargetSize(const Ptr<gpu::Texture> &texture) {
  if (!texture) {
    return validRenderSize();
  }
  const auto &info = texture->getCreateInfo();
  return Vec2(static_cast<float>(std::max<UInt32>(1, info.width)),
              static_cast<float>(std::max<UInt32>(1, info.height)));
}

static void prepareRenderPassFrame() {
  const bool depthEnabled = currentPipeline.value().getFeatureFlags().test(
      GraphicsPipeline::FeatureFlag::DepthTest);
  if (isChangedRenderTarget) {
    isChangedRenderTarget = false;
    isPrevDepthEnabled = depthEnabled;
    isFrameStarted = false;
    return;
  }

  const bool hasActivePass = (currentRenderPass != nullptr);
  const bool depthChanged = (isPrevDepthEnabled != depthEnabled);

  const bool needBegin = isFrameStarted || !hasActivePass || depthChanged;
  if (!needBegin) {
    isPrevDepthEnabled = depthEnabled;
    isFrameStarted = false;
    return;
  }

  if ((hasActivePass && depthChanged && !isFrameStarted) ||
      isChangedRenderTarget) {
    currentCommandBuffer->endRenderPass(currentRenderPass);
    isChangedRenderTarget = false;
  }

  const gpu::LoadOp loadOp = (isFrameStarted || !hasActivePass)
                                 ? gpu::LoadOp::Clear
                                 : gpu::LoadOp::Load;

  beginRenderPass(depthEnabled, loadOp);

  isPrevDepthEnabled = depthEnabled;
  isFrameStarted = false;
}

void setupShapes() {
  box.loadBox();
  sprite.loadSprite();
}
void Graphics::begin2D() {
  currentGraphicsPass = GraphicsPass::TwoD;
  currentCamera2D = std::nullopt;
  currentCamera3D = std::nullopt;
}
void Graphics::begin2D(const Camera2D &camera) {
  currentGraphicsPass = GraphicsPass::TwoD;
  currentCamera2D = camera;
  currentCamera3D = std::nullopt;
}
void Graphics::begin3D(const Camera3D &camera) {
  currentGraphicsPass = GraphicsPass::ThreeD;
  currentCamera2D = std::nullopt;
  currentCamera3D = camera;
}
void Graphics::finish() {
  currentGraphicsPass = GraphicsPass::TwoD;
  currentCamera2D = std::nullopt;
  currentCamera3D = std::nullopt;
  currentTextureBindings.clear();
  currentAccelerationStructureBindings.clear();
  currentPipeline = std::nullopt;
}
Vec2 Graphics::windowToCurrent2D(const Vec2 &windowPosition) {
  const Camera2D windowCamera(Window::size());
  const Camera2D *camera2D =
      currentCamera2D.has_value() ? &currentCamera2D.value() : &windowCamera;
  return windowPosition * camera2D->invWindowRatio();
}
void Graphics::setGraphicsPipeline(const GraphicsPipeline &pipeline) {
  customPipeline = pipeline;
}
void Graphics::resetGraphicsPipeline() { customPipeline = std::nullopt; }
void Graphics::setUniformBuffer(UInt32 slotIndex, const Buffer &buffer) {
  currentCommandBuffer->pushVertexUniformData(slotIndex, buffer.data(),
                                              buffer.size());
  currentCommandBuffer->pushFragmentUniformData(slotIndex, buffer.data(),
                                                buffer.size());
}
void Graphics::setTexture(UInt32 slotIndex, const Ptr<Texture> &texture) {
  currentTextureBindings.insert_or_assign(slotIndex, texture);
}
void Graphics::resetTexture(UInt32 slotIndex) {
  currentTextureBindings.erase(slotIndex);
}
void Graphics::resetAllTexture() { currentTextureBindings.clear(); }

void Graphics::setAccelerationStructure(
    UInt32 slotIndex,
    const Ptr<gpu::AccelerationStructure> &accelerationStructure) {
  currentAccelerationStructureBindings.insert_or_assign(slotIndex,
                                                        accelerationStructure);
}

void Graphics::resetAccelerationStructure(UInt32 slotIndex) {
  currentAccelerationStructureBindings.erase(slotIndex);
}

void Graphics::resetAllAccelerationStructures() {
  currentAccelerationStructureBindings.clear();
}

void Graphics::beginRenderTarget(const RenderTexture &texture) {
  auto tex = texture.getTexture();
  if (tex == currentColorTargets[0].texture) {
    return;
  }
  isChangedRenderTarget = true;
  auto depthTex = texture.getDepthStencil();
  currentCommandBuffer = device->acquireCommandBuffer({GlobalAllocator::get()});
  currentColorTargets[0].loadOp = gpu::LoadOp::Clear;
  currentColorTargets[0].texture = tex;
  currentDepthStencilInfo.texture = depthTex;
  currentRenderPass = currentCommandBuffer->beginRenderPass(
      currentColorTargets, currentDepthStencilInfo);
  currentRenderPass->setViewport(
      gpu::Viewport{0, 0, (float)texture.width, (float)texture.height, 0, 1});
  currentRenderPass->setScissor(0, 0, (float)texture.width,
                                (float)texture.height);
}
void Graphics::endRenderTarget() {
  currentCommandBuffer->endRenderPass(currentRenderPass);
  currentRenderPass = nullptr;
  device->submitCommandBuffer(currentCommandBuffer);
  device->waitForGpuIdle();
  currentCommandBuffer = mainCommandBuffer;
}
bool Graphics::readbackTexture(const RenderTexture &srcRenderTexture,
                               Ptr<Texture> &out) {
  SDL_assert(srcRenderTexture.width == out->size().x &&
             srcRenderTexture.height == out->size().y);
  auto tex = srcRenderTexture.getTexture();
  auto outTex = out->getRaw();
  const auto srcFormat = tex->getCreateInfo().format;
  const auto dstFormat = outTex->getCreateInfo().format;
  const auto width = static_cast<UInt32>(srcRenderTexture.width);
  const auto height = static_cast<UInt32>(srcRenderTexture.height);
  if (srcFormat == dstFormat) {
    gpu::CommandBuffer::CreateInfo info{};
    info.allocator = GlobalAllocator::get();
    auto commandBuffer = device->acquireCommandBuffer(info);
    {
      auto copyPass = commandBuffer->beginCopyPass();
      gpu::TextureLocation src{};
      src.texture = tex;
      gpu::TextureLocation dst{};
      dst.texture = outTex;
      copyPass->copyTexture(src, dst, width, height, 1, false);
      commandBuffer->endCopyPass(copyPass);
    }
    device->submitCommandBuffer(commandBuffer);
    device->waitForGpuIdle();
    currentRenderPass = nullptr;
    return true;
  }

  if (!isRgba8Format(srcFormat) || !isRgba8Format(dstFormat)) {
    Log::error("readbackTexture failed: unsupported texture format conversion");
    return false;
  }

  const UInt32 tightBytesPerRow = width * 4;
  const UInt32 downloadBytesPerRow = alignTo(tightBytesPerRow, 256);
  const size_t dataSize = static_cast<size_t>(tightBytesPerRow) * height;
  const size_t downloadSize = static_cast<size_t>(downloadBytesPerRow) * height;
  gpu::TransferBuffer::CreateInfo downloadInfo{};
  downloadInfo.allocator = GlobalAllocator::get();
  downloadInfo.size = static_cast<UInt32>(downloadSize);
  downloadInfo.usage = gpu::TransferBufferUsage::Download;
  auto downloadBuffer = device->createTransferBuffer(downloadInfo);
  {
    gpu::CommandBuffer::CreateInfo info{};
    info.allocator = GlobalAllocator::get();
    auto commandBuffer = device->acquireCommandBuffer(info);
    {
      auto copyPass = commandBuffer->beginCopyPass();
      gpu::TextureRegion src{};
      src.texture = tex;
      src.width = width;
      src.height = height;
      src.depth = 1;
      gpu::TextureTransferInfo dst{};
      dst.transferBuffer = downloadBuffer;
      copyPass->downloadTexture(src, dst);
      commandBuffer->endCopyPass(copyPass);
    }
    device->submitCommandBuffer(commandBuffer);
    device->waitForGpuIdle();
  }

  auto *mapped = static_cast<const UInt8 *>(downloadBuffer->map(false));
  if (!mapped) {
    Log::error("readbackTexture failed: download buffer map failed");
    return false;
  }
  std::vector<UInt8> converted(dataSize);
  copyTextureBytesWithFormatConversion(mapped, converted.data(), width, height,
                                       downloadBytesPerRow, tightBytesPerRow,
                                       srcFormat, dstFormat);
  downloadBuffer->unmap();

  gpu::TransferBuffer::CreateInfo uploadInfo{};
  uploadInfo.allocator = GlobalAllocator::get();
  uploadInfo.size = static_cast<UInt32>(dataSize);
  uploadInfo.usage = gpu::TransferBufferUsage::Upload;
  auto uploadBuffer = device->createTransferBuffer(uploadInfo);
  auto *uploadData = uploadBuffer->map(true);
  if (!uploadData) {
    Log::error("readbackTexture failed: upload buffer map failed");
    return false;
  }
  memcpy(uploadData, converted.data(), dataSize);
  uploadBuffer->unmap();
  {
    gpu::CommandBuffer::CreateInfo info{};
    info.allocator = GlobalAllocator::get();
    auto commandBuffer = device->acquireCommandBuffer(info);
    {
      auto copyPass = commandBuffer->beginCopyPass();
      gpu::TextureTransferInfo src{};
      src.transferBuffer = uploadBuffer;
      gpu::TextureRegion dst{};
      dst.texture = outTex;
      dst.width = width;
      dst.height = height;
      dst.depth = 1;
      copyPass->uploadTexture(src, dst, false);
      commandBuffer->endCopyPass(copyPass);
    }
    device->submitCommandBuffer(commandBuffer);
    device->waitForGpuIdle();
  }

  currentRenderPass = nullptr;
  return true;
}
void Graphics::setClearColor(const Color &color) {
  if (color.r >= 0.f && color.g >= 0.f && color.b >= 0.f)
    clearColor = color;
}
Color Graphics::getClearColor() { return clearColor; }
void Graphics::addPreDrawFunc(std::function<void()> f) {
  preDrawFuncs.push_back(f);
}
void Graphics::addPostDrawFunc(std::function<void()> function) {
  postDrawFuncs.push_back(function);
}
Ptr<gpu::Device> Graphics::getDevice() { return device; }
} // namespace sinen

namespace sinen {} // namespace sinen
