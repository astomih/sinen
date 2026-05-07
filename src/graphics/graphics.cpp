#include "graphics.hpp"
#include <cassert>
#include <core/allocator/global_allocator.hpp>
#include <gpu/gpu.hpp>
#include <gpu/shader/builtin_shader.hpp>
#include <graphics/builtin_pipeline.hpp>
#include <graphics/font/default/mplus-1p-medium.ttf.hpp>
#include <graphics/font/font_glyph_ranges.hpp>
#include <graphics/graphics.hpp>
#include <graphics/texture/render_texture.hpp>
#include <math/transform/transform.hpp>
#include <platform/io/asset_io.hpp>
#include <platform/window/window.hpp>
#include <script/luaapi.hpp>
#include <script/script.hpp>

#include <SDL3/SDL.h>

#include <cstring>

namespace sinen {
// Variables
static Color clearColor = Palette::black();
static Camera3D camera;
static Camera2D camera2D;
static bool showImGui = false;
static std::list<std::function<void()>> imguiFunctions;
static std::list<std::function<void()>> preDrawFuncs;
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
static Model box = Model();
static Model sprite = Model();

// Functions
static void prepareRenderPassFrame();
static void setupShapes();
static void beginRenderPass(bool depthEnabled, gpu::LoadOp loadOp);

static GPUBackendAPI selectBackendAPI() {
#ifdef SINEN_PLATFORM_EMSCRIPTEN
  return GPUBackendAPI::WebGPU;
#else
  return GPUBackendAPI::D3D12;
  const char *backendName = SDL_getenv("SINEN_GPU_BACKEND");
  if (backendName == nullptr || backendName[0] == '\0') {
    return GPUBackendAPI::SDLGPU;
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
  return GPUBackendAPI::SDLGPU;
#endif
}

static void setFullWindowViewport(const Ptr<gpu::RenderPass> &renderPass) {
  Rect rect;
  // SDL_Rect safeArea;
  // SDL_GetWindowSafeArea(WindowSystem::get_sdl_window(), &safeArea);
  rect.x = 0;
  rect.y = 0;
  rect.width = Window::size().x;
  rect.height = Window::size().y;

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
  camera2D = Window::size();
  camera = []() {
    Camera3D c;
    c.lookat(Vec3{0, -1, 1}, Vec3{0, 0, 0}, Vec3{0, 0, 1});
    c.perspective(90.f, Window::size().x / Window::size().y, .1f, 100.f);
    return c;
  }();
  backend = gpu::RHI::createBackend(GlobalAllocator::get(), selectBackendAPI());
  if (!backend)
    return false;
  gpu::Device::CreateInfo info{};
  info.allocator = GlobalAllocator::get();
  info.debugMode = true;
  device = backend->createDevice(info);
  if (!device)
    return false;

  auto *window = Window::getSdlWindow();
  device->claimWindow(window);
  BuiltinShader::initialize();
  BuiltinPipeline::initialize();

  // Create depth stencil target
  {
    gpu::Texture::CreateInfo depthStencilCreateInfo{};
    depthStencilCreateInfo.allocator = GlobalAllocator::get();
    depthStencilCreateInfo.width = static_cast<uint32_t>(Window::size().x);
    depthStencilCreateInfo.height = static_cast<uint32_t>(Window::size().y);
    depthStencilCreateInfo.layerCountOrDepth = 1;
    depthStencilCreateInfo.type = gpu::TextureType::Texture2D;
    depthStencilCreateInfo.usage = gpu::TextureUsage::DepthStencilTarget;
    depthStencilCreateInfo.format = gpu::TextureFormat::D32_FLOAT_S8_UINT;
    depthStencilCreateInfo.numLevels = 1;
    depthStencilCreateInfo.sampleCount = gpu::SampleCount::x1;
    depthTexture = device->createTexture(depthStencilCreateInfo);
    if (!depthTexture)
      return false;
  }

  // Default sampler
  gpu::Sampler::CreateInfo samplerInfo{};
  samplerInfo.allocator = GlobalAllocator::get();
  samplerInfo.minFilter = gpu::Filter::Linear;
  samplerInfo.magFilter = gpu::Filter::Linear;
  samplerInfo.addressModeU = gpu::AddressMode::Repeat;
  samplerInfo.addressModeV = gpu::AddressMode::Repeat;
  samplerInfo.maxAnisotropy = 1.f;
  sampler = device->createSampler(samplerInfo);
  if (!sampler)
    return false;

  gpu::Sampler::CreateInfo fontSamplerInfo = samplerInfo;
  fontSamplerInfo.minFilter = gpu::Filter::Linear;
  fontSamplerInfo.magFilter = gpu::Filter::Linear;
  fontSamplerInfo.mipmapMode = gpu::MipmapMode::Nearest;
  fontSamplerInfo.addressModeU = gpu::AddressMode::ClampToEdge;
  fontSamplerInfo.addressModeV = gpu::AddressMode::ClampToEdge;
  fontSamplerInfo.addressModeW = gpu::AddressMode::ClampToEdge;
  fontSampler = device->createSampler(fontSamplerInfo);
  if (!fontSampler)
    return false;

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
  {
    auto funcs = std::move(preDrawFuncs);
    preDrawFuncs.clear();
    for (auto &f : funcs) {
      f();
    }
  }

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
  if (Window::resized()) {
    gpu::Texture::CreateInfo depthStencilCreateInfo{};
    depthStencilCreateInfo.allocator = GlobalAllocator::get();
    depthStencilCreateInfo.width = Window::size().x;
    depthStencilCreateInfo.height = Window::size().y;
    depthStencilCreateInfo.layerCountOrDepth = 1;
    depthStencilCreateInfo.type = gpu::TextureType::Texture2D;
    depthStencilCreateInfo.usage = gpu::TextureUsage::DepthStencilTarget;
    depthStencilCreateInfo.format = gpu::TextureFormat::D32_FLOAT_S8_UINT;
    depthStencilCreateInfo.numLevels = 1;
    depthStencilCreateInfo.sampleCount = gpu::SampleCount::x1;
    depthTexture = device->createTexture(depthStencilCreateInfo);
    depthStencilInfo.texture = depthTexture;
  }
  currentDepthStencilInfo = depthStencilInfo;

  isFrameStarted = true;
  drawCallCountPerFrame = 0;
  Script::drawScene();

  // Rendering

  if (drawCallCountPerFrame == 0) {
    // Clear screen
    beginRenderPass(true, gpu::LoadOp::Clear);
  }
  commandBuffer->endRenderPass(currentRenderPass);

  device->submitCommandBuffer(commandBuffer);
  device->waitForGpuIdle();
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

static void bindCurrentTextureSamplers(
    const Ptr<gpu::RenderPass> &renderPass,
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

static void drawBase2D(const Array<Transform2D> &transforms, const Model &model,
                       const Ptr<gpu::Sampler> &textureSampler = sampler) {
  assert(currentPipeline.has_value());
  auto vertexBufferBindings = Array<gpu::BufferBinding>{};
  auto indexBufferBinding = gpu::BufferBinding{};
  auto textureSamplers = Array<gpu::TextureSamplerBinding>{};
  auto ratio = camera2D.windowRatio();
  auto invRatio = camera2D.invWindowRatio();
  Mat4 mat[3];
  Array<Mat4> instanceData;
  auto pos = transforms[0].position * ratio;
  auto scale = transforms[0].scale * 0.5f * ratio;
  {
    Transform transform;
    transform.setPosition(Vec3(pos.x, pos.y, 0.0f));
    transform.setRotation(Vec3(0, 0, transforms[0].rotation));
    transform.setScale(Vec3(scale.x, scale.y, 1.0f));

    mat[0] = transform.getWorldMatrix();
  }
  auto viewproj = Mat4(1.0f);

  auto screenSize = camera2D.size();
  viewproj[0][0] = 2.f / Window::size().x;
  viewproj[1][1] = 2.f / Window::size().y;
  mat[1] = viewproj;
  mat[2] = Mat4(1.f);
  if (transforms.size() > 1) {
    for (auto &i : transforms) {
      Transform transform;
      transform.setPosition(
          Vec3(i.position.x * ratio.x, i.position.y * ratio.y, 0.0f));
      transform.setRotation(Vec3(0, 0, i.rotation));
      transform.setScale(Vec3(i.scale.x * 0.5f, i.scale.y * 0.5f, 1.0f));
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
  renderPass->bindVertexBuffers(0, vertexBufferBindings);
  renderPass->bindIndexBuffer(indexBufferBinding,
                              gpu::IndexElementSize::Uint32);

  commandBuffer->pushVertexUniformData(0, &mat, sizeof(Mat4) * 3);
  renderPass->drawIndexedPrimitives(model.getMesh().data()->indices.size(), 1,
                                    0, 0, 0);
  currentPipeline = std::nullopt;
}

static void drawBase3D(const Array<Transform> transforms, const Model &model) {
  assert(currentPipeline.has_value());
  auto vertexBufferBindings = Array<gpu::BufferBinding>();
  auto indexBufferBinding = gpu::BufferBinding{};
  Mat4 mat[3];
  Array<Mat4> instanceData;
  {
    mat[0] = transforms[0].getWorldMatrix();
    mat[1] = camera.getView();
    mat[2] = camera.getProjection();
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
  indexBufferBinding =
      gpu::BufferBinding{.buffer = model.indexBuffer, .offset = 0};
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
  renderPass->bindVertexBuffers(0, vertexBufferBindings);
  renderPass->bindIndexBuffer(indexBufferBinding,
                              gpu::IndexElementSize::Uint32);

  commandBuffer->pushVertexUniformData(0, &mat, sizeof(Mat4) * 3);
  uint32_t numIndices = model.getMesh().data()->indices.size();
  uint32_t numInstance = isInstance ? instanceSize : 1;
  renderPass->drawIndexedPrimitives(numIndices, numInstance, 0, 0, 0);
  currentPipeline = std::nullopt;
}
void Graphics::drawRect(const Rect &rect, const Color &color, float angle) {
  if (customPipeline.has_value() && customPipeline.value().get() != nullptr)
    currentPipeline = customPipeline.value();
  else
    currentPipeline = BuiltinPipeline::getRect2D();
  Array<Transform2D> transforms(1, {rect.position(), angle, rect.size()});
  currentCommandBuffer->pushFragmentUniformData(1, &color, sizeof(Color));
  drawBase2D(transforms, sprite);
}
void Graphics::drawImage(const Ptr<Texture> &texture, const Rect &rect,
                         float angle) {
  if (customPipeline.has_value() && customPipeline.value().get() != nullptr)
    currentPipeline = customPipeline.value();
  else
    currentPipeline = BuiltinPipeline::getDefault2D();
  Array<Transform2D> transforms(1, {rect.position(), angle, rect.size()});
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
  Array<Transform2D> transforms(
      1, {position, angle, Vec2(textSize / static_cast<float>(font.size()))});
  setTexture(0, textData.texture);
  const Vec2 atlasSize = textData.texture->size();
  const FontFragmentParams params{color,
                                  Vec4(atlasSize.x, atlasSize.y, 6.0f, 0.0f)};
  currentCommandBuffer->pushFragmentUniformData(1, &params, sizeof(params));

  drawBase2D(transforms, model, fontSampler);
}
void Graphics::drawCubemap(const Ptr<Texture> &cubemap) {
  if (customPipeline.has_value() && customPipeline.value().get() != nullptr)
    currentPipeline = customPipeline.value();
  else
    currentPipeline = BuiltinPipeline::getCubemap();

  setTexture(0, cubemap);
  Array<Transform> transforms(1, Transform());
  drawBase3D(transforms, box);
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
  // Copy
  gpu::TransferBuffer::CreateInfo info2{};
  info2.allocator = GlobalAllocator::get();
  info2.size = srcRenderTexture.width * srcRenderTexture.height * 4;
  info2.usage = gpu::TransferBufferUsage::Download;
  auto transferBuffer = device->createTransferBuffer(info2);
  {
    gpu::CommandBuffer::CreateInfo info{};
    info.allocator = GlobalAllocator::get();
    auto commandBuffer = device->acquireCommandBuffer(info);
    {
      auto copyPass = commandBuffer->beginCopyPass();
      {
        gpu::TextureRegion region{};
        region.width = srcRenderTexture.width;
        region.height = srcRenderTexture.height;
        region.depth = 1;
        region.texture = tex;
        gpu::TextureTransferInfo dst{};
        dst.offset = 0;
        dst.transferBuffer = transferBuffer;
        copyPass->downloadTexture(region, dst);
      }
      {
        gpu::TextureTransferInfo src{};
        src.offset = 0;
        src.transferBuffer = transferBuffer;
        gpu::TextureRegion region{};
        region.x = 0;
        region.y = 0;
        region.width = srcRenderTexture.width;
        region.height = srcRenderTexture.height;
        region.depth = 1;
        region.texture = out->getRaw();
        copyPass->uploadTexture(src, region, false);
      }
      commandBuffer->endCopyPass(copyPass);
    }
    device->submitCommandBuffer(commandBuffer);
    device->waitForGpuIdle();
  }
  currentRenderPass = nullptr;
  return true;
}
void Graphics::setCamera2D(const Camera2D &camera) { camera2D = camera; }
Camera2D &Graphics::getCamera2D() { return camera2D; }
void Graphics::setCamera3D(const Camera3D &_camera) { camera = _camera; }
Camera3D &Graphics::getCamera() { return camera; }
void Graphics::setClearColor(const Color &color) {
  if (color.r >= 0.f && color.g >= 0.f && color.b >= 0.f)
    clearColor = color;
}
Color Graphics::getClearColor() { return clearColor; }
void Graphics::toggleShowImGui() { showImGui = !showImGui; }
bool Graphics::isShowImGui() { return showImGui; }
std::list<std::function<void()>> &Graphics::getImGuiFunction() {
  return imguiFunctions;
}
void Graphics::addPreDrawFunc(std::function<void()> f) {
  preDrawFuncs.push_back(f);
}
void Graphics::addImGuiFunction(std::function<void()> function) {
  imguiFunctions.push_back(function);
}
Ptr<gpu::Device> Graphics::getDevice() { return device; }
} // namespace sinen

namespace sinen {

static int lGraphicsDrawRect(lua_State *L) {
  auto &rect = udValue<Rect>(L, 1);
  auto &color = udValue<Color>(L, 2);
  if (lua_gettop(L) >= 3) {
    float angle = static_cast<float>(luaL_checknumber(L, 3));
    Graphics::drawRect(rect, color, angle);
    return 0;
  }
  Graphics::drawRect(rect, color);
  return 0;
}
static int lGraphicsDrawText(lua_State *L) {
  const char *text = luaL_checkstring(L, 1);
  auto &font = udPtr<Font>(L, 2);
  auto &pos = udValue<Vec2>(L, 3);
  if (lua_gettop(L) == 3) {
    Graphics::drawText(StringView(text), *font, pos);
    return 0;
  }
  auto &color = udValue<Color>(L, 4);
  float size = static_cast<float>(luaL_optnumber(L, 5, 32.0));
  float angle = static_cast<float>(luaL_optnumber(L, 6, 0.0));
  Graphics::drawText(StringView(text), *font, pos, color, size, angle);
  return 0;
}
static int lGraphicsDrawImage(lua_State *L) {
  auto &tex = udPtr<Texture>(L, 1);
  auto &rect = udValue<Rect>(L, 2);
  if (lua_gettop(L) >= 3) {
    float angle = static_cast<float>(luaL_checknumber(L, 3));
    Graphics::drawImage(tex, rect, angle);
    return 0;
  }
  Graphics::drawImage(tex, rect);
  return 0;
}
static int lGraphicsDrawCubemap(lua_State *L) {
  auto &tex = udPtr<Texture>(L, 1);
  Graphics::drawCubemap(tex);
  return 0;
}
static int lGraphicsDrawModel(lua_State *L) {
  auto &m = udPtr<Model>(L, 1);
  auto &t = udValue<Transform>(L, 2);
  Graphics::drawModel(*m, t);
  return 0;
}
static int lGraphicsDrawModelInstanced(lua_State *L) {
  auto &m = udPtr<Model>(L, 1);
  luaL_checktype(L, 2, LUA_TTABLE);
  Array<Transform> transforms;
  size_t n = lua_objlen(L, 2);
  transforms.reserve(n);
  for (size_t i = 1; i <= n; ++i) {
    lua_rawgeti(L, 2, static_cast<lua_Integer>(i));
    transforms.push_back(udValue<Transform>(L, -1));
    lua_pop(L, 1);
  }
  Graphics::drawModelInstanced(*m, transforms);
  return 0;
}
static int lGraphicsSetCamera3D(lua_State *L) {
  auto &cam = udValue<Camera3D>(L, 1);
  Graphics::setCamera3D(cam);
  return 0;
}
static int lGraphicsGetCamera3D(lua_State *L) {
  auto &cam = Graphics::getCamera();
  udNewRef<Camera3D>(L, &cam);
  return 1;
}
static int lGraphicsSetCamera2D(lua_State *L) {
  auto &cam = udValue<Camera2D>(L, 1);
  Graphics::setCamera2D(cam);
  return 0;
}
static int lGraphicsGetCamera2D(lua_State *L) {
  auto &cam = Graphics::getCamera2D();
  udNewRef<Camera2D>(L, &cam);
  return 1;
}
static int lGraphicsGetClearColor(lua_State *L) {
  udNewOwned<Color>(L, Graphics::getClearColor());
  return 1;
}
static int lGraphicsSetClearColor(lua_State *L) {
  auto &c = udValue<Color>(L, 1);
  Graphics::setClearColor(c);
  return 0;
}
static int lGraphicsSetGraphicsPipeline(lua_State *L) {
  auto &p = udPtr<GraphicsPipeline>(L, 1);
  Graphics::setGraphicsPipeline(*p);
  return 0;
}
static int lGraphicsResetGraphicsPipeline(lua_State *L) {
  (void)L;
  Graphics::resetGraphicsPipeline();
  return 0;
}
static int lGraphicsSetTexture(lua_State *L) {
  UInt32 slot = static_cast<UInt32>(luaL_checkinteger(L, 1));
  auto &t = udPtr<Texture>(L, 2);
  Graphics::setTexture(slot, t);
  return 0;
}
static int lGraphicsResetTexture(lua_State *L) {
  UInt32 slot = static_cast<UInt32>(luaL_checkinteger(L, 1));
  Graphics::resetTexture(slot);
  return 0;
}
static int lGraphicsResetAllTexture(lua_State *L) {
  (void)L;
  Graphics::resetAllTexture();
  return 0;
}
static int lGraphicsSetUniformBuffer(lua_State *L) {
  UInt32 slot = static_cast<UInt32>(luaL_checkinteger(L, 1));
  auto &b = udValue<Buffer>(L, 2);
  Graphics::setUniformBuffer(slot, b);
  return 0;
}
static int lGraphicsBeginRenderTarget(lua_State *L) {
  auto &rt = udPtr<RenderTexture>(L, 1);
  Graphics::beginRenderTarget(*rt);
  return 0;
}
static int lGraphicsEndRenderTarget(lua_State *L) {
  (void)L;
  Graphics::endRenderTarget();
  return 0;
}
static int lGraphicsReadbackTexture(lua_State *L) {
  auto &rt = udPtr<RenderTexture>(L, 1);
  auto &out = udPtr<Texture>(L, 2);
  lua_pushboolean(L, Graphics::readbackTexture(*rt, out));
  return 1;
}
void registerGraphics(lua_State *L) {
  pushSnNamed(L, "Graphics");
  luaPushcfunction2(L, lGraphicsDrawRect);
  lua_setfield(L, -2, "drawRect");
  luaPushcfunction2(L, lGraphicsDrawText);
  lua_setfield(L, -2, "drawText");
  luaPushcfunction2(L, lGraphicsDrawImage);
  lua_setfield(L, -2, "drawImage");
  luaPushcfunction2(L, lGraphicsDrawCubemap);
  lua_setfield(L, -2, "drawCubemap");
  luaPushcfunction2(L, lGraphicsDrawModel);
  lua_setfield(L, -2, "drawModel");
  luaPushcfunction2(L, lGraphicsDrawModelInstanced);
  lua_setfield(L, -2, "drawModelInstanced");
  luaPushcfunction2(L, lGraphicsSetCamera3D);
  lua_setfield(L, -2, "setCamera3D");
  luaPushcfunction2(L, lGraphicsGetCamera3D);
  lua_setfield(L, -2, "getCamera3D");
  luaPushcfunction2(L, lGraphicsSetCamera2D);
  lua_setfield(L, -2, "setCamera2D");
  luaPushcfunction2(L, lGraphicsGetCamera2D);
  lua_setfield(L, -2, "getCamera2D");
  luaPushcfunction2(L, lGraphicsGetClearColor);
  lua_setfield(L, -2, "getClearColor");
  luaPushcfunction2(L, lGraphicsSetClearColor);
  lua_setfield(L, -2, "setClearColor");
  luaPushcfunction2(L, lGraphicsSetGraphicsPipeline);
  lua_setfield(L, -2, "setGraphicsPipeline");
  luaPushcfunction2(L, lGraphicsResetGraphicsPipeline);
  lua_setfield(L, -2, "resetGraphicsPipeline");
  luaPushcfunction2(L, lGraphicsSetTexture);
  lua_setfield(L, -2, "setTexture");
  luaPushcfunction2(L, lGraphicsResetTexture);
  lua_setfield(L, -2, "resetTexture");
  luaPushcfunction2(L, lGraphicsResetAllTexture);
  lua_setfield(L, -2, "resetAllTexture");
  luaPushcfunction2(L, lGraphicsSetUniformBuffer);
  lua_setfield(L, -2, "setUniformBuffer");
  luaPushcfunction2(L, lGraphicsBeginRenderTarget);
  lua_setfield(L, -2, "beginRenderTarget");
  luaPushcfunction2(L, lGraphicsEndRenderTarget);
  lua_setfield(L, -2, "endRenderTarget");
  luaPushcfunction2(L, lGraphicsReadbackTexture);
  lua_setfield(L, -2, "readbackTexture");
  lua_pop(L, 1);
}

} // namespace sinen
