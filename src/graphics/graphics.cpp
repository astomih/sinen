#include "graphics.hpp"
#include <asset/texture/render_texture.hpp>
#include <cassert>
#include <core/allocator/global_allocator.hpp>
#include <graphics/graphics.hpp>
#include <graphics/imgui_backend/imgui_impl_sinen.hpp>
#include <memory>
#include <platform/io/asset_io.hpp>
#include <platform/window/window.hpp>

#include <asset/font/default/mplus-1p-medium.ttf.hpp>
#include <asset/script/script.hpp>
#include <asset/shader/builtin_shader.hpp>
#include <graphics/builtin_pipeline.hpp>
#include <graphics/rhi/rhi.hpp>
#include <platform/window/window.hpp>

#include <SDL3/SDL.h>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_internal.h>

namespace sinen {
static Color clearColor = Palette::black();
static Camera camera;
static Camera2D camera2D;
static bool showImGui = false;
static std::list<std::function<void()>> imguiFunctions;
static std::list<std::function<void()>> preDrawFuncs;
static Ptr<rhi::Backend> backend;
static Ptr<rhi::Device> device;
static Ptr<rhi::Texture> depthTexture;
static Ptr<rhi::Sampler> sampler;
static std::optional<GraphicsPipeline> currentPipeline;
static std::optional<GraphicsPipeline> customPipeline;
static Ptr<rhi::CommandBuffer> mainCommandBuffer;
static Ptr<rhi::CommandBuffer> currentCommandBuffer;
static Ptr<rhi::RenderPass> currentRenderPass;
static bool isFrameStarted = true;
static bool isPrevDepthEnabled = true;
static bool isChangedRenderTarget = false;
static uint32_t drawCallCountPerFrame = 0;
static Array<rhi::ColorTargetInfo> colorTargets = Array<rhi::ColorTargetInfo>();
static rhi::DepthStencilTargetInfo depthStencilInfo;
static Array<rhi::ColorTargetInfo> currentColorTargets;
static rhi::DepthStencilTargetInfo currentDepthStencilInfo;
static Hashmap<UInt32, rhi::TextureSamplerBinding> currentTextureBindings;
void setFullWindowViewport(const Ptr<rhi::RenderPass> &renderPass) {
  Rect rect;
  // SDL_Rect safeArea;
  // SDL_GetWindowSafeArea(WindowSystem::get_sdl_window(), &safeArea);
  rect.x = 0;
  rect.y = 0;
  rect.width = Window::size().x;
  rect.height = Window::size().y;

  rhi::Viewport viewport{};
  viewport.x = rect.x;
  viewport.y = rect.y;
  viewport.width = rect.width;
  viewport.height = rect.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  renderPass->setViewport(viewport);
  renderPass->setScissor(rect.x, rect.y, rect.width, rect.height);
}
Model Graphics::box = Model();
Model Graphics::sprite = Model();
bool Graphics::initialize() {
  camera2D = Window::size();
  camera = []() {
    Camera c;
    c.lookat(Vec3{0, -1, 1}, Vec3{0, 0, 0}, Vec3{0, 0, 1});
    c.perspective(90.f, Window::size().x / Window::size().y, .1f, 100.f);
    return c;
  }();
  backend =
      rhi::RHI::createBackend(GlobalAllocator::get(), rhi::GraphicsAPI::SDLGPU);
  if (!backend)
    return false;
  rhi::Device::CreateInfo info{};
  info.allocator = GlobalAllocator::get();
  info.debugMode = true;
  device = backend->createDevice(info);
  if (!device)
    return false;

  auto *window = Window::getSdlWindow();
  device->claimWindow(window);
  IMGUI_CHECKVERSION();
  auto *context = ImGui::CreateContext();
#ifdef __ANDROID__
  float sizePixels = 64.0f;
#else
  float sizePixels = 32.0f;
#endif
  context->IO.Fonts->AddFontFromMemoryTTF(
      (void *)mplus1pMediumTtf, mplus1pMediumTtfLen, sizePixels, nullptr,
      context->IO.Fonts->GetGlyphRangesJapanese());
  context->IO.IniFilename = nullptr;
  context->IO.WantTextInput = true;
  auto *imeData = &context->PlatformImeData;
  imeData->WantVisible = true;
  ImGui_ImplSDL3_InitForSDLGPU(window);
  ImGuiImplParanoixaInitInfo initInfo = {};
  initInfo.Allocator = GlobalAllocator::get();
  initInfo.Device = device;
  initInfo.ColorTargetFormat = device->getSwapchainFormat();
  initInfo.MSAASamples = rhi::SampleCount::x1;
  imGuiImplParanoixaInit(&initInfo);

  BuiltinShader::initialize();
  BuiltinPipeline::initialize();

  // Create depth stencil target
  {
    rhi::Texture::CreateInfo depthStencilCreateInfo{};
    depthStencilCreateInfo.allocator = GlobalAllocator::get();
    depthStencilCreateInfo.width = static_cast<uint32_t>(Window::size().x);
    depthStencilCreateInfo.height = static_cast<uint32_t>(Window::size().y);
    depthStencilCreateInfo.layerCountOrDepth = 1;
    depthStencilCreateInfo.type = rhi::TextureType::Texture2D;
    depthStencilCreateInfo.usage = rhi::TextureUsage::DepthStencilTarget;
    depthStencilCreateInfo.format = rhi::TextureFormat::D32_FLOAT_S8_UINT;
    depthStencilCreateInfo.numLevels = 1;
    depthStencilCreateInfo.sampleCount = rhi::SampleCount::x1;
    depthTexture = device->createTexture(depthStencilCreateInfo);
    if (!depthTexture)
      return false;
  }

  // Default sampler
  rhi::Sampler::CreateInfo samplerInfo{};
  samplerInfo.allocator = GlobalAllocator::get();
  samplerInfo.minFilter = rhi::Filter::Linear;
  samplerInfo.magFilter = rhi::Filter::Linear;
  samplerInfo.addressModeU = rhi::AddressMode::Repeat;
  samplerInfo.addressModeV = rhi::AddressMode::Repeat;
  samplerInfo.maxAnisotropy = 1.f;
  sampler = device->createSampler(samplerInfo);
  if (!sampler)
    return false;

  colorTargets.push_back({});
  depthStencilInfo.texture = depthTexture;
  depthStencilInfo.loadOp = rhi::LoadOp::Clear;
  depthStencilInfo.storeOp = rhi::StoreOp::Store;
  depthStencilInfo.clearDepth = 1.0f;
  depthStencilInfo.clearStencil = 0;
  depthStencilInfo.cycle = false;
  depthStencilInfo.stencilLoadOp = rhi::LoadOp::Clear;
  depthStencilInfo.stencilStoreOp = rhi::StoreOp::Store;
  setupShapes();
  return true;
}

void Graphics::shutdown() {
  device.reset();
  backend.reset();
}

void Graphics::render() {
  for (auto &f : preDrawFuncs) {
    f();
  }
  preDrawFuncs.clear();

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
    rhi::Texture::CreateInfo depthStencilCreateInfo{};
    depthStencilCreateInfo.allocator = GlobalAllocator::get();
    depthStencilCreateInfo.width = Window::size().x;
    depthStencilCreateInfo.height = Window::size().y;
    depthStencilCreateInfo.layerCountOrDepth = 1;
    depthStencilCreateInfo.type = rhi::TextureType::Texture2D;
    depthStencilCreateInfo.usage = rhi::TextureUsage::DepthStencilTarget;
    depthStencilCreateInfo.format = rhi::TextureFormat::D32_FLOAT_S8_UINT;
    depthStencilCreateInfo.numLevels = 1;
    depthStencilCreateInfo.sampleCount = rhi::SampleCount::x1;
    depthTexture = device->createTexture(depthStencilCreateInfo);
    depthStencilInfo.texture = depthTexture;
  }
  currentDepthStencilInfo = depthStencilInfo;

  imGuiImplParanoixaNewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();
  for (auto &func : Graphics::getImGuiFunction()) {
    func();
  }

  isFrameStarted = true;
  drawCallCountPerFrame = 0;
  Script::drawScene();

  // Rendering
  ImGui::Render();
  ImDrawData *drawData = ImGui::GetDrawData();

  if (drawCallCountPerFrame == 0) {
    // Clear screen
    beginRenderPass(true, rhi::LoadOp::Clear);
  }
  commandBuffer->endRenderPass(currentRenderPass);

  imGuiImplParanoixaPrepareDrawData(drawData, commandBuffer);

  beginRenderPass(false, rhi::LoadOp::Load);
  // Render ImGui
  imGuiImplParanoixaRenderDrawData(drawData, commandBuffer, currentRenderPass);
  commandBuffer->endRenderPass(currentRenderPass);

  device->submitCommandBuffer(commandBuffer);
  device->waitForGpuIdle();
}
void Graphics::drawBase2D(const sinen::Draw2D &draw2D) {
  assert(currentPipeline.has_value());
  auto vertexBufferBindings = Array<rhi::BufferBinding>{};
  auto indexBufferBinding = rhi::BufferBinding{};
  auto textureSamplers = Array<rhi::TextureSamplerBinding>{};
  auto ratio = camera2D.windowRatio();
  auto invRatio = camera2D.invWindowRatio();
  Mat4 mat[3];
  Array<Mat4> instanceData;
  auto pos = draw2D.position * ratio;
  auto scale = draw2D.scale * 0.5f * ratio;
  {
    Transform transform;
    transform.setPosition(Vec3(pos.x, pos.y, 0.0f));
    transform.setRotation(Vec3(0, 0, draw2D.rotation));
    transform.setScale(Vec3(scale.x, scale.y, 1.0f));

    mat[0] = transform.getWorldMatrix();
  }
  auto viewproj = Mat4(1.0f);

  auto screenSize = camera2D.size();
  viewproj[0][0] = 2.f / Window::size().x;
  viewproj[1][1] = 2.f / Window::size().y;
  mat[1] = viewproj;
  mat[2] = Mat4(1.f);
  for (auto &i : draw2D.worlds) {
    Transform transform;
    transform.setPosition(
        Vec3(i.position.x * ratio.x, i.position.y * ratio.y, 0.0f));
    transform.setRotation(Vec3(0, 0, i.rotation));
    transform.setScale(Vec3(i.scale.x * 0.5f, i.scale.y * 0.5f, 1.0f));
    instanceData.push_back(transform.getWorldMatrix());
  }
  drawCallCountPerFrame++;
  prepareRenderPassFrame();

  const auto &model = draw2D.model;
  assert(model.vertexBuffer != nullptr);
  assert(model.indexBuffer != nullptr);

  vertexBufferBindings.emplace_back(
      rhi::BufferBinding{.buffer = model.vertexBuffer, .offset = 0});
  indexBufferBinding =
      rhi::BufferBinding{.buffer = model.indexBuffer, .offset = 0};

  auto commandBuffer = currentCommandBuffer;
  auto renderPass = currentRenderPass;
  renderPass->bindGraphicsPipeline(currentPipeline.value().get());
  for (auto &i : currentTextureBindings) {
    renderPass->bindFragmentSampler(i.first, i.second);
  }
  renderPass->bindVertexBuffers(0, vertexBufferBindings);
  renderPass->bindIndexBuffer(indexBufferBinding,
                              rhi::IndexElementSize::Uint32);

  commandBuffer->pushUniformData(0, &mat, sizeof(Mat4) * 3);
  renderPass->drawIndexedPrimitives(model.getMesh().data()->indices.size(), 1,
                                    0, 0, 0);
  currentPipeline = std::nullopt;
}

void Graphics::drawBase3D(const sinen::Draw3D &draw3D) {
  assert(currentPipeline.has_value());
  auto vertexBufferBindings = Array<rhi::BufferBinding>();
  auto indexBufferBinding = rhi::BufferBinding{};
  Mat4 mat[3];
  Array<Mat4> instanceData;
  {
    Transform transform;
    transform.setPosition(draw3D.position);
    transform.setRotation(draw3D.rotation);
    transform.setScale(draw3D.scale);
    mat[0] = transform.getWorldMatrix();
    mat[1] = camera.getView();
    mat[2] = camera.getProjection();
  }
  for (auto &i : draw3D.worlds) {
    instanceData.push_back(i.getWorldMatrix());
  }
  drawCallCountPerFrame++;
  prepareRenderPassFrame();

  auto instanceSize = sizeof(Mat4) * instanceData.size();
  bool isInstance = instanceSize > 0;
  Ptr<rhi::Buffer> instanceBuffer = nullptr;
  if (isInstance) {
    if (!currentPipeline.has_value())
      currentPipeline = BuiltinPipeline::getInstanced3D();
    rhi::Buffer::CreateInfo instanceBufferInfo{};
    instanceBufferInfo.allocator = GlobalAllocator::get();
    instanceBufferInfo.size = instanceSize;
    instanceBufferInfo.usage = rhi::BufferUsage::Vertex;
    instanceBuffer = device->createBuffer(instanceBufferInfo);
    Ptr<rhi::TransferBuffer> transferBuffer;
    {
      rhi::TransferBuffer::CreateInfo info{};
      info.allocator = GlobalAllocator::get();
      info.size = instanceSize;
      info.usage = rhi::TransferBufferUsage::Upload;
      transferBuffer = device->createTransferBuffer(info);
      auto *pMapped = transferBuffer->map(false);
      if (pMapped) {
        memcpy(pMapped, instanceData.data(), instanceSize);
      }
      transferBuffer->unmap();
    }
    {
      rhi::CommandBuffer::CreateInfo info{};
      info.allocator = GlobalAllocator::get();
      auto commandBuffer = device->acquireCommandBuffer(info);
      {
        auto copyPass = commandBuffer->beginCopyPass();
        {
          rhi::BufferTransferInfo src{};
          src.offset = 0;
          src.transferBuffer = transferBuffer;
          rhi::BufferRegion dst{};
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

  const auto &model = draw3D.getModel();
  vertexBufferBindings.emplace_back(
      rhi::BufferBinding{.buffer = model.vertexBuffer, .offset = 0});
  indexBufferBinding =
      rhi::BufferBinding{.buffer = model.indexBuffer, .offset = 0};
  if (isInstance) {
    vertexBufferBindings.emplace_back(
        rhi::BufferBinding{.buffer = instanceBuffer, .offset = 0});
  }
  if (auto animationVertexBuffer = model.animationVertexBuffer) {
    vertexBufferBindings.emplace_back(
        rhi::BufferBinding{.buffer = animationVertexBuffer, .offset = 0});
  }
  if (model.tangentBuffer) {
    vertexBufferBindings.emplace_back(
        rhi::BufferBinding{.buffer = model.tangentBuffer, .offset = 0});
  }
  auto commandBuffer = currentCommandBuffer;
  auto renderPass = currentRenderPass;
  renderPass->bindGraphicsPipeline(currentPipeline.value().get());
  for (auto &i : currentTextureBindings) {
    renderPass->bindFragmentSampler(i.first, i.second);
  }
  renderPass->bindVertexBuffers(0, vertexBufferBindings);
  renderPass->bindIndexBuffer(indexBufferBinding,
                              rhi::IndexElementSize::Uint32);

  commandBuffer->pushUniformData(0, &mat, sizeof(Mat4) * 3);
  uint32_t numIndices = model.getMesh().data()->indices.size();
  uint32_t numInstance = isInstance ? instanceSize : 1;
  renderPass->drawIndexedPrimitives(numIndices, numInstance, 0, 0, 0);
  currentPipeline = std::nullopt;
}
void Graphics::drawRect(const Rect &rect, const Color &color, float angle) {
  if (customPipeline.has_value())
    currentPipeline = customPipeline.value();
  else
    currentPipeline = BuiltinPipeline::getDefault2D();
  sinen::Draw2D draw2D;
  draw2D.position = rect.position();
  draw2D.scale = rect.size();

  draw2D.rotation = angle;
  Texture texture;
  texture.fill(color);
  setTexture(0, texture);
  Graphics::drawBase2D(draw2D);
}
void Graphics::drawImage(const Texture &texture, const Rect &rect,
                         float angle) {
  if (customPipeline.has_value())
    currentPipeline = customPipeline.value();
  else
    currentPipeline = BuiltinPipeline::getDefault2D();
  sinen::Draw2D draw2D;
  draw2D.position = rect.position();
  draw2D.scale = rect.size();

  draw2D.rotation = angle;
  setTexture(0, texture);
  Graphics::drawBase2D(draw2D);
}
void Graphics::drawText(StringView text, const Font &font, const Vec2 &position,
                        const Color &color, float textSize, float angle) {
  if (customPipeline.has_value())
    currentPipeline = customPipeline.value();
  else
    currentPipeline = BuiltinPipeline::getDefault2D();
  sinen::Draw2D draw2D;
  Model model;
  model.loadFromVertexArray(font.getTextMesh(text));

  draw2D.position = position;
  Texture texture = font.getAtlas();
  draw2D.rotation = angle;
  setTexture(0, texture);
  draw2D.model = model;
  draw2D.scale = Vec2(textSize / static_cast<float>(font.size()));
  Graphics::drawBase2D(draw2D);
}
void Graphics::drawCubemap(const Texture &cubemap) {
  if (customPipeline.has_value())
    currentPipeline = customPipeline.value();
  else
    currentPipeline = BuiltinPipeline::getCubemap();
  Transform transform;
  Model model;
  model.loadBox();
  setTexture(0, cubemap);
  sinen::Draw3D draw3D;
  draw3D.position = transform.position;
  draw3D.scale = transform.scale;
  draw3D.rotation = transform.rotation;
  draw3D.model = model;
  Graphics::drawBase3D(draw3D);
}
void Graphics::drawModel(const Model &model, const Transform &transform) {
  if (customPipeline.has_value())
    currentPipeline = customPipeline.value();
  else
    currentPipeline = BuiltinPipeline::getDefault3D();

  if (model.hasTexture(TextureKey::BaseColor))
    setTexture(0, model.getTexture(TextureKey::BaseColor));
  else {
    Texture t;
    t.fill(Palette::white());
    setTexture(0, t);
  }

  sinen::Draw3D draw3D;
  draw3D.position = transform.position;
  draw3D.scale = transform.scale;
  draw3D.rotation = transform.rotation;
  draw3D.model = model;
  Graphics::drawBase3D(draw3D);
}
void Graphics::drawModelInstanced(const Model &model,
                                  const Array<Transform> &transforms) {
  if (customPipeline.has_value())
    currentPipeline = customPipeline.value();
  else
    currentPipeline = BuiltinPipeline::getInstanced3D();
  if (model.hasTexture(TextureKey::BaseColor))
    setTexture(0, model.getTexture(TextureKey::BaseColor));
  else {
    Texture t;
    t.fill(Palette::white());
    setTexture(0, t);
  }
  sinen::Draw3D draw3D;
  draw3D.position = Vec3{0, 0, 0};
  draw3D.scale = Vec3{1, 1, 1};
  draw3D.rotation = Vec3{0, 0, 0};
  draw3D.model = model;
  draw3D.worlds = transforms;
  Graphics::drawBase3D(draw3D);
}

void Graphics::beginRenderPass(bool depthEnabled, rhi::LoadOp loadOp) {
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

void Graphics::prepareRenderPassFrame() {
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

  const rhi::LoadOp loadOp = (isFrameStarted || !hasActivePass)
                                 ? rhi::LoadOp::Clear
                                 : rhi::LoadOp::Load;

  beginRenderPass(depthEnabled, loadOp);

  isPrevDepthEnabled = depthEnabled;
  isFrameStarted = false;
}

void Graphics::setupShapes() {
  AABB aabb;
  box.loadFromVertexArray(aabb.createMesh());
  Rect rect;
  sprite.loadFromVertexArray(rect.createMesh());
}
void Graphics::setGraphicsPipeline(const GraphicsPipeline &pipeline) {
  customPipeline = pipeline;
}
void Graphics::resetGraphicsPipeline() { customPipeline = std::nullopt; }
void Graphics::setUniformBuffer(UInt32 slotIndex, const Buffer &buffer) {
  currentCommandBuffer->pushUniformData(slotIndex, buffer.data(),
                                        buffer.size());
}
void Graphics::setTexture(UInt32 slotIndex, const Texture &texture) {
  currentTextureBindings.insert_or_assign(
      slotIndex, rhi::TextureSamplerBinding{.sampler = sampler,
                                            .texture = texture.texture});
}
void Graphics::resetTexture(UInt32 slotIndex) {
  currentTextureBindings.erase(slotIndex);
}
void Graphics::resetAllTexture() { currentTextureBindings.clear(); }

void Graphics::setRenderTarget(const RenderTexture &texture) {
  auto tex = texture.getTexture();
  if (tex == currentColorTargets[0].texture) {
    return;
  }
  isChangedRenderTarget = true;
  auto depthTex = texture.getDepthStencil();
  currentCommandBuffer = device->acquireCommandBuffer({GlobalAllocator::get()});
  currentColorTargets[0].loadOp = rhi::LoadOp::Clear;
  currentColorTargets[0].texture = tex;
  currentDepthStencilInfo.texture = depthTex;
  currentRenderPass = currentCommandBuffer->beginRenderPass(
      currentColorTargets, currentDepthStencilInfo);
  currentRenderPass->setViewport(
      rhi::Viewport{0, 0, (float)texture.width, (float)texture.height, 0, 1});
  currentRenderPass->setScissor(0, 0, (float)texture.width,
                                (float)texture.height);
}
void Graphics::flush() {
  currentCommandBuffer->endRenderPass(currentRenderPass);
  device->submitCommandBuffer(currentCommandBuffer);
  device->waitForGpuIdle();
  currentCommandBuffer = mainCommandBuffer;
}
bool Graphics::readbackTexture(const RenderTexture &srcRenderTexture,
                               Texture &out) {
  auto tex = srcRenderTexture.getTexture();
  // Copy
  rhi::TransferBuffer::CreateInfo info2{};
  info2.allocator = GlobalAllocator::get();
  info2.size = srcRenderTexture.width * srcRenderTexture.height * 4;
  info2.usage = rhi::TransferBufferUsage::Download;
  auto transferBuffer = device->createTransferBuffer(info2);
  {
    rhi::CommandBuffer::CreateInfo info{};
    info.allocator = GlobalAllocator::get();
    auto commandBuffer = device->acquireCommandBuffer(info);
    {
      auto copyPass = commandBuffer->beginCopyPass();
      {
        rhi::TextureRegion region{};
        region.width = srcRenderTexture.width;
        region.height = srcRenderTexture.height;
        region.depth = 1;
        region.texture = tex;
        rhi::TextureTransferInfo dst{};
        dst.offset = 0;
        dst.transferBuffer = transferBuffer;
        copyPass->downloadTexture(region, dst);
      }
      {
        rhi::TextureTransferInfo src{};
        src.offset = 0;
        src.transferBuffer = transferBuffer;
        rhi::TextureRegion region{};
        region.x = 0;
        region.y = 0;
        region.width = srcRenderTexture.width;
        region.height = srcRenderTexture.height;
        region.depth = 1;
        region.texture = out.texture;
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
void Graphics::setCamera(const Camera &_camera) { camera = _camera; }
Camera &Graphics::getCamera() { return camera; }
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
Ptr<rhi::Device> Graphics::getDevice() { return device; }
} // namespace sinen
