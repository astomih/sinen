#include "graphics_system.hpp"

#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>

#include "core/allocator/global_allocator.hpp"
#include "imgui_backend/imgui_impl_sinen.hpp"
#include <asset/model/mesh.hpp>
#include <asset/texture/render_texture.hpp>
#include <cassert>
#include <core/io/asset_io.hpp>
#include <core/logger/logger.hpp>
#include <graphics/graphics.hpp>
#include <memory>
#include <platform/window/window.hpp>

#include "../asset/font/default/mplus-1p-medium.ttf.hpp"
#include "../asset/model/default_model_creator.hpp"
#include "../asset/model/model_data.hpp"
#include "../asset/script/script_system.hpp"
#include "../asset/texture/texture_data.hpp"
#include "../main_system.hpp"
#include "../platform/window/window_system.hpp"
#include "imgui_internal.h"
#include <graphics/rhi/rhi.hpp>

namespace sinen {
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
Color GraphicsSystem::clearColor = Palette::black();
// Renderer
bool GraphicsSystem::showImGui = false;
std::list<std::function<void()>> GraphicsSystem::imguiFunctions;
Model GraphicsSystem::box = Model();
Model GraphicsSystem::sprite = Model();
void GraphicsSystem::initialize() {
  camera2D = Window::size();
  camera = []() {
    Camera c;
    c.lookat(glm::vec3{0, -1, 1}, glm::vec3{0, 0, 0}, glm::vec3{0, 0, 1});
    c.perspective(90.f, Window::size().x / Window::size().y, .1f, 100.f);
    return c;
  }();
  backend = rhi::RHI::createBackend(gA, rhi::GraphicsAPI::SDLGPU);
  rhi::Device::CreateInfo info{};
  info.allocator = gA;
  info.debugMode = true;
  device = backend->createDevice(info);
  auto *window = WindowSystem::getSdlWindow();
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
  initInfo.Allocator = gA;
  initInfo.Device = device;
  initInfo.ColorTargetFormat = device->getSwapchainFormat();
  initInfo.MSAASamples = rhi::SampleCount::x1;
  imGuiImplParanoixaInit(&initInfo);

  Shader vs;
  vs.loadDefaultVertexShader();
  Shader vsInstanced;
  vsInstanced.loadDefaultVertexInstanceShader();
  Shader fs;
  fs.loadDefaultFragmentShader();

  pipeline3D.setVertexShader(vs);
  pipeline3D.setFragmentShader(fs);
  pipeline3D.setEnableDepthTest(true);
  pipeline3D.build();

  pipelineInstanced3D.setVertexShader(vsInstanced);
  pipelineInstanced3D.setFragmentShader(fs);
  pipelineInstanced3D.setEnableInstanced(true);
  pipelineInstanced3D.setEnableDepthTest(true);
  pipelineInstanced3D.build();

  pipeline2D.setVertexShader(vs);
  pipeline2D.setFragmentShader(fs);
  pipeline2D.setEnableDepthTest(false);
  pipeline2D.build();
  currentPipeline = pipeline2D;

  // Create depth stencil target
  {
    rhi::Texture::CreateInfo depthStencilCreateInfo{};
    depthStencilCreateInfo.allocator = gA;
    depthStencilCreateInfo.width = static_cast<uint32_t>(Window::size().x);
    depthStencilCreateInfo.height = static_cast<uint32_t>(Window::size().y);
    depthStencilCreateInfo.layerCountOrDepth = 1;
    depthStencilCreateInfo.type = rhi::TextureType::Texture2D;
    depthStencilCreateInfo.usage = rhi::TextureUsage::DepthStencilTarget;
    depthStencilCreateInfo.format = rhi::TextureFormat::D32_FLOAT_S8_UINT;
    depthStencilCreateInfo.numLevels = 1;
    depthStencilCreateInfo.sampleCount = rhi::SampleCount::x1;
    depthTexture = device->createTexture(depthStencilCreateInfo);
  }

  // Default sampler
  rhi::Sampler::CreateInfo samplerInfo{};
  samplerInfo.allocator = gA;
  samplerInfo.minFilter = rhi::Filter::Linear;
  samplerInfo.magFilter = rhi::Filter::Linear;
  samplerInfo.addressModeU = rhi::AddressMode::Repeat;
  samplerInfo.addressModeV = rhi::AddressMode::Repeat;
  samplerInfo.maxAnisotropy = 1.f;
  sampler = device->createSampler(samplerInfo);

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
}

void GraphicsSystem::shutdown() {
  device.reset();
  backend.reset();
}

void GraphicsSystem::render() {
  auto commandBuffer = device->acquireCommandBuffer({gA});
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
  if (WindowSystem::resized()) {
    rhi::Texture::CreateInfo depthStencilCreateInfo{};
    depthStencilCreateInfo.allocator = gA;
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
  for (auto &func : GraphicsSystem::getImGuiFunction()) {
    func();
  }
  // Rendering
  ImGui::Render();
  ImDrawData *drawData = ImGui::GetDrawData();

  imGuiImplParanoixaPrepareDrawData(drawData, commandBuffer);
  isFrameStarted = true;
  drawCallCountPerFrame = 0;
  ScriptSystem::drawScene();
  if (drawCallCountPerFrame == 0) {
    // Clear screen
    beginRenderPass(true, rhi::LoadOp::Clear);
  }
  commandBuffer->endRenderPass(currentRenderPass);

  if (showImGui) {
    beginRenderPass(false, rhi::LoadOp::Load);
    // Render ImGui
    imGuiImplParanoixaRenderDrawData(drawData, commandBuffer,
                                     currentRenderPass);
    commandBuffer->endRenderPass(currentRenderPass);
  }
  device->submitCommandBuffer(commandBuffer);
  device->waitForGpuIdle();
}
void GraphicsSystem::drawBase2D(const sinen::Draw2D &draw2D) {
  auto vertexBufferBindings = Array<rhi::BufferBinding>{};
  auto indexBufferBinding = rhi::BufferBinding{};
  auto textureSamplers = Array<rhi::TextureSamplerBinding>{};
  auto ratio = camera2D.windowRatio();
  glm::mat4 mat[3];
  std::vector<glm::mat4> instanceData;
  auto scale = draw2D.scale * 0.5f * ratio;
  {
    auto t = glm::translate(glm::mat4(1.0f),
                            glm::vec3(draw2D.position.x * ratio.x,
                                      draw2D.position.y * ratio.y, 0.0f));
    auto quaternion = glm::angleAxis(glm::radians(draw2D.rotation),
                                     glm::vec3(0.0f, 0.0f, -1.0f));
    auto r = glm::toMat4(quaternion);

    auto s = glm::scale(glm::mat4(1.0f), glm::vec3(scale, 1.0f));

    mat[0] = glm::transpose(t * r * s);
  }
  auto viewproj = glm::mat4(1.0f);

  auto screenSize = camera2D.size();
  viewproj[0][0] = 2.f / Window::size().x;
  viewproj[1][1] = 2.f / Window::size().y;
  mat[1] = glm::transpose(viewproj);
  mat[2] = glm::mat4(1.f);
  for (auto &i : draw2D.worlds) {
    auto t = glm::translate(
        glm::mat4(1.0f),
        glm::vec3(i.position.x * ratio.x, i.position.y * ratio.y, 0.0f));
    auto quaternion =
        glm::angleAxis(glm::radians(i.rotation), glm::vec3(0.0f, 0.0f, -1.0f));
    auto r = glm::toMat4(quaternion);
    auto s = glm::scale(glm::mat4(1.0f),
                        glm::vec3(i.scale.x * 0.5f, i.scale.y * 0.5f, 1.0f));

    auto world = t * r * s;

    instanceData.push_back(glm::transpose(world));
  }
  drawCallCountPerFrame++;
  prepareRenderPassFrame();

  for (const auto &texture : draw2D.material.getTextures()) {
    auto nativeTexture = texture.texture;
    textureSamplers.push_back(rhi::TextureSamplerBinding{
        .sampler = sampler, .texture = nativeTexture});
  }

  const auto &model = draw2D.model;
  assert(model.vertexBuffer != nullptr);
  assert(model.indexBuffer != nullptr);

  vertexBufferBindings.emplace_back(
      rhi::BufferBinding{.buffer = model.vertexBuffer, .offset = 0});
  indexBufferBinding =
      rhi::BufferBinding{.buffer = model.indexBuffer, .offset = 0};

  auto commandBuffer = currentCommandBuffer;
  auto renderPass = currentRenderPass;
  renderPass->bindGraphicsPipeline(currentPipeline.get());
  renderPass->bindFragmentSamplers(0, textureSamplers);
  renderPass->bindVertexBuffers(0, vertexBufferBindings);
  renderPass->bindIndexBuffer(indexBufferBinding,
                              rhi::IndexElementSize::Uint32);

  commandBuffer->pushUniformData(0, &mat, sizeof(glm::mat4) * 3);
  renderPass->drawIndexedPrimitives(model.getMesh().indexCount, 1, 0, 0, 0);
}

void GraphicsSystem::drawBase3D(const sinen::Draw3D &draw3D) {
  auto vertexBufferBindings = Array<rhi::BufferBinding>();
  auto indexBufferBinding = rhi::BufferBinding{};
  auto textureSamplers = Array<rhi::TextureSamplerBinding>();
  glm::mat4 mat[3];
  std::vector<glm::mat4> instanceData;
  {
    const auto t = glm::translate(
        glm::mat4(1.0f),
        glm::vec3(draw3D.position.x, draw3D.position.y, draw3D.position.z));
    const auto rotationX = glm::angleAxis(glm::radians(draw3D.rotation.x),
                                          glm::vec3(1.0f, 0.0f, 0.0f));
    const auto rotationY = glm::angleAxis(glm::radians(draw3D.rotation.y),
                                          glm::vec3(0.0f, 1.0f, 0.0f));
    const auto rotationZ = glm::angleAxis(glm::radians(draw3D.rotation.z),
                                          glm::vec3(0.0f, 0.0f, 1.0f));
    const auto r = glm::toMat4(rotationX * rotationY * rotationZ);

    const auto s =
        glm::scale(glm::mat4(1.0f),
                   glm::vec3(draw3D.scale.x, draw3D.scale.y, draw3D.scale.z));
    mat[0] = glm::transpose(t * r * s);
    mat[1] = glm::transpose(camera.getView());
    mat[2] = glm::transpose(camera.getProjection());
  }
  for (auto &i : draw3D.worlds) {
    auto t = glm::translate(
        glm::mat4(1.0f), glm::vec3(i.position.x, i.position.y, i.position.z));
    auto rotationX =
        glm::angleAxis(glm::radians(i.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    auto rotationY =
        glm::angleAxis(glm::radians(i.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    auto rotationZ =
        glm::angleAxis(glm::radians(i.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    auto r = glm::toMat4(rotationX * rotationY * rotationZ);

    auto s =
        glm::scale(glm::mat4(1.0f), glm::vec3(i.scale.x, i.scale.y, i.scale.z));

    auto world = t * r * s;
    instanceData.push_back(glm::transpose(world));
  }
  drawCallCountPerFrame++;
  prepareRenderPassFrame();

  for (const auto &texture : draw3D.material.getTextures()) {
    auto nativeTexture = texture.texture;
    textureSamplers.push_back(rhi::TextureSamplerBinding{
        .sampler = sampler, .texture = nativeTexture});
  }

  for (const auto &cubemap : draw3D.material.getCubemaps()) {
    auto nativeCubemap = cubemap.getNativeCubemap();
    textureSamplers.push_back(rhi::TextureSamplerBinding{
        .sampler = sampler, .texture = nativeCubemap});
  }

  auto instanceSize = sizeof(glm::mat4) * instanceData.size();
  bool isInstance = instanceSize > 0;
  Ptr<rhi::Buffer> instanceBuffer = nullptr;
  if (isInstance) {
    rhi::Buffer::CreateInfo instanceBufferInfo{};
    instanceBufferInfo.allocator = gA;
    instanceBufferInfo.size = instanceSize;
    instanceBufferInfo.usage = rhi::BufferUsage::Vertex;
    instanceBuffer = device->createBuffer(instanceBufferInfo);
    Ptr<rhi::TransferBuffer> transferBuffer;
    {
      rhi::TransferBuffer::CreateInfo info{};
      info.allocator = gA;
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
      info.allocator = gA;
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
  renderPass->bindGraphicsPipeline(currentPipeline.get());
  renderPass->bindFragmentSamplers(0, textureSamplers);
  renderPass->bindVertexBuffers(0, vertexBufferBindings);
  renderPass->bindIndexBuffer(indexBufferBinding,
                              rhi::IndexElementSize::Uint32);

  commandBuffer->pushUniformData(0, &mat, sizeof(glm::mat4) * 3);
  uint32_t numIndices = model.getMesh().indexCount;
  uint32_t numInstance = isInstance ? instanceSize : 1;
  renderPass->drawIndexedPrimitives(numIndices, numInstance, 0, 0, 0);
}
void GraphicsSystem::drawRect(const Rect &rect, const Color &color,
                              float angle) {
  sinen::Draw2D draw2D;
  draw2D.position = rect.p;
  draw2D.scale = rect.s;

  draw2D.rotation = angle;
  draw2D.material = Material();
  Texture texture;
  texture.fill(color);
  draw2D.material.setTexture(texture);
  GraphicsSystem::drawBase2D(draw2D);
}
void GraphicsSystem::drawImage(const Texture &texture, const Rect &rect,
                               float angle) {
  sinen::Draw2D draw2D;
  draw2D.position = rect.p;
  draw2D.scale = rect.s;

  draw2D.rotation = angle;
  draw2D.material = Material();
  draw2D.material.setTexture(texture);
  GraphicsSystem::drawBase2D(draw2D);
}
void GraphicsSystem::drawText(const std::string &text, const Font &font,
                              const glm::vec2 &position, const Color &color,
                              float textSize, float angle) {
  sinen::Draw2D draw2D;
  Model model;
  model.loadFromVertexArray(font.getTextMesh(text));

  draw2D.position = position;
  Texture texture = font.getAtlas();
  draw2D.rotation = angle;
  draw2D.material = Material();
  draw2D.material.setTexture(texture);
  draw2D.model = model;
  draw2D.scale = glm::vec2(textSize / static_cast<float>(font.size()));
  GraphicsSystem::drawBase2D(draw2D);
}
void GraphicsSystem::drawCubemap(const Cubemap &cubemap) {

  Transform transform;
  Model model;
  model.loadBox();
  Material material;
  material.setCubemap(cubemap);
  sinen::Draw3D draw3D;
  draw3D.position = transform.position;
  draw3D.scale = transform.scale;
  draw3D.rotation = transform.rotation;
  draw3D.material = material;
  draw3D.model = model;
  GraphicsSystem::drawBase3D(draw3D);
}
void GraphicsSystem::drawModel(const Model &model, const Transform &transform,
                               const Material &material) {
  sinen::Draw3D draw3D;
  draw3D.position = transform.position;
  draw3D.scale = transform.scale;
  draw3D.rotation = transform.rotation;
  draw3D.material = material;
  draw3D.model = model;
  GraphicsSystem::drawBase3D(draw3D);
}
void GraphicsSystem::drawModelInstanced(
    const Model &model, const std::vector<Transform> &transforms,
    const Material &material) {
  sinen::Draw3D draw3D;
  draw3D.position = {0, 0, 0};
  draw3D.scale = {1, 1, 1};
  draw3D.rotation = {0, 0, 0};
  draw3D.material = material;
  draw3D.model = model;
  draw3D.worlds = transforms;
  GraphicsSystem::drawBase3D(draw3D);
}

void GraphicsSystem::loadShader(const Shader &shaderInfo) {}
void GraphicsSystem::unloadShader(const Shader &shaderInfo) {}
void GraphicsSystem::beginRenderPass(bool depthEnabled, rhi::LoadOp loadOp) {
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

void GraphicsSystem::prepareRenderPassFrame() {
  const bool depthEnabled = currentPipeline.getFeatureFlags().test(
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

void GraphicsSystem::setupShapes() {
  box.loadFromVertexArray(createBoxVertices());
  sprite.loadFromVertexArray(createSpriteVertices());
}

void GraphicsSystem::bindPipeline(const GraphicsPipeline &pipeline) {
  currentPipeline = pipeline;
}
void GraphicsSystem::bindDefaultPipeline3D() { currentPipeline = pipeline3D; }
void GraphicsSystem::bindDefaultPipeline2D() { currentPipeline = pipeline2D; }
void GraphicsSystem::setUniformData(uint32_t slot, const UniformData &data) {
  currentCommandBuffer->pushUniformData(slot, data.data.data(),
                                        data.data.size() * sizeof(float));
}
void GraphicsSystem::setRenderTarget(const RenderTexture &texture) {
  auto tex = texture.getTexture();
  if (tex == currentColorTargets[0].texture) {
    return;
  }
  isChangedRenderTarget = true;
  auto depthTex = texture.getDepthStencil();
  currentCommandBuffer = device->acquireCommandBuffer({gA});
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
void GraphicsSystem::flush() {
  currentCommandBuffer->endRenderPass(currentRenderPass);
  device->submitCommandBuffer(currentCommandBuffer);
  device->waitForGpuIdle();
  currentCommandBuffer = mainCommandBuffer;
}
bool GraphicsSystem::readbackTexture(const RenderTexture &srcRenderTexture,
                                     Texture &out) {
  auto tex = srcRenderTexture.getTexture();
  // Copy
  rhi::TransferBuffer::CreateInfo info2{};
  info2.allocator = gA;
  info2.size = srcRenderTexture.width * srcRenderTexture.height * 4;
  info2.usage = rhi::TransferBufferUsage::Download;
  auto transferBuffer = device->createTransferBuffer(info2);
  {
    rhi::CommandBuffer::CreateInfo info{};
    info.allocator = gA;
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
} // namespace sinen
