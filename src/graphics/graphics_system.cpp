#include "graphics_system.hpp"

#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>

#include <asset/model/mesh.hpp>
#include <asset/texture/render_texture.hpp>
#include <cassert>
#include <core/io/asset_io.hpp>
#include <core/logger/logger.hpp>
#include <graphics/graphics.hpp>
#include <imgui_impl_paranoixa.hpp>
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
#include "paranoixa/paranoixa.hpp"

namespace sinen {
void setFullWindowViewport(const px::Ptr<px::RenderPass> &renderPass) {
  Rect rect;
  // SDL_Rect safeArea;
  // SDL_GetWindowSafeArea(WindowSystem::get_sdl_window(), &safeArea);
  rect.x = 0;
  rect.y = 0;
  rect.width = Window::size().x;
  rect.height = Window::size().y;

  px::Viewport viewport{};
  viewport.x = rect.x;
  viewport.y = rect.y;
  viewport.width = rect.width;
  viewport.height = rect.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  renderPass->SetViewport(viewport);
  renderPass->SetScissor(rect.x, rect.y, rect.width, rect.height);
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
  backend = px::Paranoixa::CreateBackend(allocator, px::GraphicsAPI::SDLGPU);
  px::Device::CreateInfo info{};
  info.allocator = allocator;
  info.debugMode = true;
  device = backend->CreateDevice(info);
  auto *window = WindowSystem::get_sdl_window();
  device->ClaimWindow(window);
  IMGUI_CHECKVERSION();
  auto *context = ImGui::CreateContext();
#ifdef __ANDROID__
  float sizePixels = 64.0f;
#else
  float sizePixels = 32.0f;
#endif
  context->IO.Fonts->AddFontFromMemoryTTF(
      (void *)mplus_1p_medium_ttf, mplus_1p_medium_ttf_len, sizePixels, nullptr,
      context->IO.Fonts->GetGlyphRangesJapanese());
  context->IO.IniFilename = nullptr;
  context->IO.WantTextInput = true;
  auto *ime_data = &context->PlatformImeData;
  ime_data->WantVisible = true;
  ImGui_ImplSDL3_InitForSDLGPU(window);
  ImGui_ImplParanoixa_InitInfo init_info = {};
  init_info.Allocator = allocator;
  init_info.Device = device;
  init_info.ColorTargetFormat = device->GetSwapchainFormat();
  init_info.MSAASamples = px::SampleCount::x1;
  ImGui_ImplParanoixa_Init(&init_info);

  Shader vs;
  vs.loadDefaultVertexShader();
  Shader vsInstanced;
  vsInstanced.loadDefaultVertexInstanceShader();
  Shader fs;
  fs.loadDefaultFragmentShader();

  pipeline3D.setVertexShader(vs);
  pipeline3D.setFragmentShader(fs);
  pipeline3D.build();
  currentPipeline3D = pipeline3D;

  pipelineInstanced3D.setVertexShader(vsInstanced);
  pipelineInstanced3D.setFragmentShader(fs);
  pipelineInstanced3D.setInstanced(true);
  pipelineInstanced3D.build();

  pipeline2D.setVertexShader(vs);
  pipeline2D.setFragmentShader(fs);
  pipeline2D.build();
  currentPipeline2D = pipeline2D;

  // Create depth stencil target
  {
    px::Texture::CreateInfo depthStencilCreateInfo{};
    depthStencilCreateInfo.allocator = allocator;
    depthStencilCreateInfo.width = static_cast<uint32_t>(Window::size().x);
    depthStencilCreateInfo.height = static_cast<uint32_t>(Window::size().y);
    depthStencilCreateInfo.layerCountOrDepth = 1;
    depthStencilCreateInfo.type = px::TextureType::Texture2D;
    depthStencilCreateInfo.usage = px::TextureUsage::DepthStencilTarget;
    depthStencilCreateInfo.format = px::TextureFormat::D32_FLOAT_S8_UINT;
    depthStencilCreateInfo.numLevels = 1;
    depthStencilCreateInfo.sampleCount = px::SampleCount::x1;
    depthTexture = device->CreateTexture(depthStencilCreateInfo);
  }

  // Default sampler
  px::Sampler::CreateInfo samplerInfo{};
  samplerInfo.allocator = allocator;
  samplerInfo.minFilter = px::Filter::Nearest;
  samplerInfo.magFilter = px::Filter::Nearest;
  samplerInfo.addressModeU = px::AddressMode::Repeat;
  samplerInfo.addressModeV = px::AddressMode::Repeat;
  samplerInfo.maxAnisotropy = 1.f;
  sampler = device->CreateSampler(samplerInfo);

  colorTargets.push_back({});
  depthStencilInfo.texture = depthTexture;
  depthStencilInfo.loadOp = px::LoadOp::Clear;
  depthStencilInfo.storeOp = px::StoreOp::Store;
  depthStencilInfo.clearDepth = 1.0f;
  depthStencilInfo.clearStencil = 0;
  depthStencilInfo.cycle = false;
  depthStencilInfo.stencilLoadOp = px::LoadOp::Clear;
  depthStencilInfo.stencilStoreOp = px::StoreOp::Store;
  setupShapes();
}

void GraphicsSystem::shutdown() {
  device.reset();
  backend.reset();
}

void GraphicsSystem::render() {
  auto commandBuffer = device->AcquireCommandBuffer({allocator});
  if (commandBuffer == nullptr) {
    return;
  }
  auto swapchainTexture = device->AcquireSwapchainTexture(commandBuffer);
  if (swapchainTexture == nullptr) {
    return;
  }
  mainCommandBuffer = commandBuffer;
  currentCommandBuffer = commandBuffer;
  colorTargets[0].texture = swapchainTexture;
  currentColorTargets = colorTargets;
  if (WindowSystem::resized()) {
    px::Texture::CreateInfo depthStencilCreateInfo{};
    depthStencilCreateInfo.allocator = allocator;
    depthStencilCreateInfo.width = Window::size().x;
    depthStencilCreateInfo.height = Window::size().y;
    depthStencilCreateInfo.layerCountOrDepth = 1;
    depthStencilCreateInfo.type = px::TextureType::Texture2D;
    depthStencilCreateInfo.usage = px::TextureUsage::DepthStencilTarget;
    depthStencilCreateInfo.format = px::TextureFormat::D32_FLOAT_S8_UINT;
    depthStencilCreateInfo.numLevels = 1;
    depthStencilCreateInfo.sampleCount = px::SampleCount::x1;
    depthTexture = device->CreateTexture(depthStencilCreateInfo);
    depthStencilInfo.texture = depthTexture;
  }
  currentDepthStencilInfo = depthStencilInfo;

  ImGui_ImplParanoixa_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();
  for (auto &func : GraphicsSystem::getImGuiFunction()) {
    func();
  }
  // Rendering
  ImGui::Render();
  ImDrawData *draw_data = ImGui::GetDrawData();

  Imgui_ImplParanoixa_PrepareDrawData(draw_data, commandBuffer);
  isFrameStarted = true;
  objectCount = 0;
  ScriptSystem::DrawScene();
  if (objectCount > 0 && !isDraw2D) {
    commandBuffer->EndRenderPass(currentRenderPass);
  }

  {
    if (isFrameStarted || (objectCount > 0 && !isDraw2D)) {
      colorTargets[0].loadOp = px::LoadOp::Load;
      currentRenderPass = commandBuffer->BeginRenderPass(colorTargets, {});
    }
    auto renderPass = currentRenderPass;
    setFullWindowViewport(renderPass);
    // Render ImGui
    ImGui_ImplParanoixa_RenderDrawData(draw_data, commandBuffer, renderPass);
    commandBuffer->EndRenderPass(renderPass);
  }
  device->SubmitCommandBuffer(commandBuffer);
  device->WaitForGPUIdle();
}
void GraphicsSystem::drawBase2D(const sinen::Draw2D &draw2D) {
  auto vertexBufferBindings = px::Array<px::BufferBinding>{allocator};
  auto indexBufferBinding = px::BufferBinding{};
  auto textureSamplers = px::Array<px::TextureSamplerBinding>{allocator};
  auto ratio = camera2D.windowRatio();
  glm::mat4 mat[3];
  std::vector<glm::mat4> instanceData;
  {
    auto t = glm::translate(glm::mat4(1.0f),
                            glm::vec3(draw2D.position.x * ratio.x,
                                      draw2D.position.y * ratio.y, 0.0f));
    auto quaternion = glm::angleAxis(glm::radians(draw2D.rotation),
                                     glm::vec3(0.0f, 0.0f, -1.0f));
    auto r = glm::toMat4(quaternion);

    auto s =
        glm::scale(glm::mat4(1.0f), glm::vec3(draw2D.scale.x * 0.5f,
                                              draw2D.scale.y * 0.5f, 1.0f));

    mat[0] = glm::transpose(t * r * s);
  }
  auto viewproj = glm::mat4(1.0f);

  auto screen_size = camera2D.size();
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
  objectCount++;
  if (isFrameStarted || currentRenderPass == nullptr) {
    colorTargets[0].loadOp = px::LoadOp::Clear;
    currentColorTargets = colorTargets;
    currentRenderPass =
        currentCommandBuffer->BeginRenderPass(currentColorTargets, {});
    auto renderPass = currentRenderPass;
    setFullWindowViewport(renderPass);
    isFrameStarted = false;
    isDraw2D = true;
    isDefaultPipeline = true;
  } else if (isDefaultPipeline && !isDraw2D) {
    currentCommandBuffer->EndRenderPass(currentRenderPass);
    colorTargets[0].loadOp = px::LoadOp::Load;
    currentColorTargets = colorTargets;
    currentRenderPass =
        currentCommandBuffer->BeginRenderPass(currentColorTargets, {});
    auto renderPass = currentRenderPass;
    setFullWindowViewport(renderPass);
    isDraw2D = true;
    isDefaultPipeline = true;
  }

  for (const auto &texture : draw2D.material.getTextures()) {
    auto nativeTexture = std::static_pointer_cast<px::Texture>(
        getTextureRawData(texture.textureData)->texture);
    textureSamplers.push_back(px::TextureSamplerBinding{
        .sampler = sampler, .texture = nativeTexture});
  }

  const auto &model = draw2D.getModel();
  assert(model.vertexBuffer != nullptr);
  assert(model.indexBuffer != nullptr);

  vertexBufferBindings.emplace_back(
      px::BufferBinding{.buffer = model.vertexBuffer, .offset = 0});
  indexBufferBinding =
      px::BufferBinding{.buffer = model.indexBuffer, .offset = 0};

  auto commandBuffer = currentCommandBuffer;
  auto renderPass = currentRenderPass;
  renderPass->BindGraphicsPipeline(currentPipeline2D.get());
  renderPass->BindFragmentSamplers(0, textureSamplers);
  renderPass->BindVertexBuffers(0, vertexBufferBindings);
  renderPass->BindIndexBuffer(indexBufferBinding, px::IndexElementSize::Uint32);

  commandBuffer->PushUniformData(0, &mat, sizeof(glm::mat4) * 3);
  renderPass->DrawIndexedPrimitives(model.getMesh().indexCount, 1, 0, 0, 0);
}

void GraphicsSystem::drawBase3D(const sinen::Draw3D &draw3D) {
  auto vertexBufferBindings = px::Array<px::BufferBinding>{allocator};
  auto indexBufferBinding = px::BufferBinding{};
  auto textureSamplers = px::Array<px::TextureSamplerBinding>{allocator};
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
  objectCount++;
  if (isDefaultPipeline && (isFrameStarted || currentRenderPass == nullptr)) {
    colorTargets[0].loadOp = px::LoadOp::Clear;
    currentColorTargets = colorTargets;
    currentDepthStencilInfo = depthStencilInfo;
    currentRenderPass = currentCommandBuffer->BeginRenderPass(
        currentColorTargets, currentDepthStencilInfo);
    auto renderPass = currentRenderPass;
    setFullWindowViewport(renderPass);
    isFrameStarted = false;
    isDraw2D = false;
    isDefaultPipeline = true;
  } else if (isDefaultPipeline && isDraw2D) {
    currentCommandBuffer->EndRenderPass(currentRenderPass);
    colorTargets[0].loadOp = px::LoadOp::Load;
    currentColorTargets = colorTargets;
    currentDepthStencilInfo = depthStencilInfo;
    currentRenderPass = currentCommandBuffer->BeginRenderPass(
        currentColorTargets, currentDepthStencilInfo);
    auto renderPass = currentRenderPass;
    setFullWindowViewport(renderPass);
    isDraw2D = false;
    isDefaultPipeline = true;
  }
  for (const auto &texture : draw3D.material.getTextures()) {
    auto nativeTexture = std::static_pointer_cast<px::Texture>(
        getTextureRawData(texture.textureData)->texture);
    textureSamplers.push_back(px::TextureSamplerBinding{
        .sampler = sampler, .texture = nativeTexture});
  }

  auto instanceSize = sizeof(glm::mat4) * instanceData.size();
  bool isInstance = instanceSize > 0;
  px::Ptr<px::Buffer> instanceBuffer = nullptr;
  if (isInstance) {
    px::Buffer::CreateInfo instanceBufferInfo{};
    instanceBufferInfo.allocator = allocator;
    instanceBufferInfo.size = instanceSize;
    instanceBufferInfo.usage = px::BufferUsage::Vertex;
    instanceBuffer = device->CreateBuffer(instanceBufferInfo);
    Ptr<px::TransferBuffer> transferBuffer;
    {
      px::TransferBuffer::CreateInfo info{};
      info.allocator = allocator;
      info.size = instanceSize;
      info.usage = px::TransferBufferUsage::Upload;
      transferBuffer = device->CreateTransferBuffer(info);
      auto *pMapped = transferBuffer->Map(false);
      if (pMapped) {
        memcpy(pMapped, instanceData.data(), instanceSize);
      }
      transferBuffer->Unmap();
    }
    {
      px::CommandBuffer::CreateInfo info{};
      info.allocator = allocator;
      auto commandBuffer = device->AcquireCommandBuffer(info);
      {
        auto copyPass = commandBuffer->BeginCopyPass();
        {
          px::BufferTransferInfo src{};
          src.offset = 0;
          src.transferBuffer = transferBuffer;
          px::BufferRegion dst{};
          dst.offset = 0;
          dst.size = instanceSize;
          dst.buffer = instanceBuffer;
          copyPass->UploadBuffer(src, dst, false);
        }
        commandBuffer->EndCopyPass(copyPass);
      }
      device->SubmitCommandBuffer(commandBuffer);
    }
  }

  const auto &model = draw3D.getModel();
  vertexBufferBindings.emplace_back(
      px::BufferBinding{.buffer = model.vertexBuffer, .offset = 0});
  indexBufferBinding =
      px::BufferBinding{.buffer = model.indexBuffer, .offset = 0};
  if (model.tangentBuffer) {
    vertexBufferBindings.emplace_back(
        px::BufferBinding{.buffer = model.tangentBuffer, .offset = 0});
  }
  if (isInstance) {
    vertexBufferBindings.emplace_back(
        px::BufferBinding{.buffer = instanceBuffer, .offset = 0});
  }
  if (auto animationVertexBuffer = model.animationVertexBuffer) {
    vertexBufferBindings.emplace_back(
        px::BufferBinding{.buffer = animationVertexBuffer, .offset = 0});
  }
  auto commandBuffer = currentCommandBuffer;
  auto renderPass = currentRenderPass;
  if (isInstance)
    renderPass->BindGraphicsPipeline(pipelineInstanced3D.get());
  else
    renderPass->BindGraphicsPipeline(currentPipeline3D.get());
  renderPass->BindFragmentSamplers(0, textureSamplers);
  renderPass->BindVertexBuffers(0, vertexBufferBindings);
  renderPass->BindIndexBuffer(indexBufferBinding, px::IndexElementSize::Uint32);

  commandBuffer->PushUniformData(0, &mat, sizeof(glm::mat4) * 3);
  uint32_t numIndices = model.getMesh().indexCount;
  uint32_t numInstance = isInstance ? instanceSize : 1;
  renderPass->DrawIndexedPrimitives(numIndices, numInstance, 0, 0, 0);
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
void GraphicsSystem::drawText(const std::string &text,
                              const glm::vec2 &position, const Color &color,
                              float fontSize, float angle) {
  sinen::Draw2D draw2D;
  draw2D.position = position;
  Font font;
  font.load(fontSize);
  Texture texture;
  font.renderText(texture, text, color);
  draw2D.scale = texture.size();
  draw2D.rotation = angle;
  draw2D.material = Material();
  draw2D.material.setTexture(texture);
  GraphicsSystem::drawBase2D(draw2D);
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

void GraphicsSystem::setupShapes() {
  box.loadFromVertexArray(create_box_vertices());
  sprite.loadFromVertexArray(create_sprite_vertices());
}

void GraphicsSystem::bindPipeline3D(const GraphicsPipeline3D &pipeline) {
  currentPipeline3D = pipeline;
}
void GraphicsSystem::bindDefaultPipeline3D() { currentPipeline3D = pipeline3D; }
void GraphicsSystem::bindPipeline2D(const GraphicsPipeline2D &pipeline) {
  currentPipeline2D = pipeline;
}
void GraphicsSystem::bindDefaultPipeline2D() { currentPipeline2D = pipeline2D; }
void GraphicsSystem::setUniformData(uint32_t slot, const UniformData &data) {
  currentCommandBuffer->PushUniformData(slot, data.data.data(),
                                        data.data.size() * sizeof(float));
}
void GraphicsSystem::setRenderTarget(const RenderTexture &texture) {
  auto tex = texture.getTexture();
  auto depthTex = texture.getDepthStencil();
  currentCommandBuffer = device->AcquireCommandBuffer({allocator});
  currentColorTargets[0].loadOp = px::LoadOp::Clear;
  currentColorTargets[0].texture = tex;
  currentDepthStencilInfo.texture = depthTex;
  currentRenderPass = currentCommandBuffer->BeginRenderPass(
      currentColorTargets, currentDepthStencilInfo);
  currentRenderPass->SetViewport(
      px::Viewport{0, 0, (float)texture.width, (float)texture.height, 0, 1});
  currentRenderPass->SetScissor(0, 0, (float)texture.width,
                                (float)texture.height);
  isDefaultPipeline = false;
}
void GraphicsSystem::flush() {
  currentCommandBuffer->EndRenderPass(currentRenderPass);
  device->SubmitCommandBuffer(currentCommandBuffer);
  device->WaitForGPUIdle();
  currentCommandBuffer = mainCommandBuffer;
}
bool GraphicsSystem::readbackTexture(const RenderTexture &srcRenderTexture,
                                     Texture &out) {
  auto tex = srcRenderTexture.getTexture();
  auto outTextureData = getTextureRawData(out.textureData);
  // Copy
  px::TransferBuffer::CreateInfo info2{};
  info2.allocator = allocator;
  info2.size = srcRenderTexture.width * srcRenderTexture.height * 4;
  info2.usage = px::TransferBufferUsage::Download;
  auto transferBuffer = device->CreateTransferBuffer(info2);
  {
    px::CommandBuffer::CreateInfo info{};
    info.allocator = allocator;
    auto commandBuffer = device->AcquireCommandBuffer(info);
    {
      auto copyPass = commandBuffer->BeginCopyPass();
      {
        px::TextureRegion region{};
        region.width = srcRenderTexture.width;
        region.height = srcRenderTexture.height;
        region.depth = 1;
        region.texture = tex;
        px::TextureTransferInfo dst{};
        dst.offset = 0;
        dst.transferBuffer = transferBuffer;
        copyPass->DownloadTexture(region, dst);
      }
      {
        px::TextureTransferInfo src{};
        src.offset = 0;
        src.transferBuffer = transferBuffer;
        px::TextureRegion region{};
        region.x = 0;
        region.y = 0;
        region.width = srcRenderTexture.width;
        region.height = srcRenderTexture.height;
        region.depth = 1;
        region.texture = outTextureData->texture;
        copyPass->UploadTexture(src, region, false);
      }
      commandBuffer->EndCopyPass(copyPass);
    }
    device->SubmitCommandBuffer(commandBuffer);
    device->WaitForGPUIdle();
  }
  currentRenderPass = nullptr;
  return true;
}
} // namespace sinen
