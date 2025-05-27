#include "px_renderer.hpp"

#include "../platform/window/window_system.hpp"
#include "imgui_internal.h"
#include "paranoixa/paranoixa.hpp"
#include "render/renderer.hpp"
#include "render_system.hpp"
#include <SDL3/SDL.h>
#include <cassert>
#include <core/io/data_stream.hpp>
#include <memory>
#include <platform/window/window.hpp>
#include <render/drawable/instance_data.hpp>

#include <imgui.h>
#include <imgui_impl_paranoixa.hpp>
#include <imgui_impl_sdl3.h>

#include "../asset/model/model_data.hpp"
#include "../asset/texture/texture_data.hpp"
#include "../logic/script/script_system.hpp"

#include "../logic/scene/scene_system.hpp"
#include <asset/texture/render_texture.hpp>

// TODO:
// - Refactoring
// - Shadow mapping

namespace sinen {
PxDrawable::PxDrawable(px::Allocator *allocator)
    : allocator(allocator), vertexBuffers(allocator),
      textureSamplers(allocator) {}
PxRenderer::PxRenderer(px::Allocator *allocator)
    : allocator(allocator), vertexArrays(allocator), colorTargets(allocator),
      currentColorTargets(allocator) {}
void PxRenderer::initialize() {
  backend = px::Paranoixa::CreateBackend(allocator, px::GraphicsAPI::SDLGPU);
  px::Device::CreateInfo info{};
  info.allocator = allocator;
  info.debugMode = true;
  device = backend->CreateDevice(info);
  auto *window = WindowImpl::get_sdl_window();
  device->ClaimWindow(window);
  IMGUI_CHECKVERSION();
  auto *context = ImGui::CreateContext();
  auto *ime_data = &context->PlatformImeData;
  ime_data->WantVisible = true;
  ImGui_ImplSDL3_InitForSDLGPU(window);
  ImGui_ImplParanoixa_InitInfo init_info = {};
  init_info.Allocator = allocator;
  init_info.Device = device;
  init_info.ColorTargetFormat = px::TextureFormat::B8G8R8A8_UNORM;
  init_info.MSAASamples = px::SampleCount::x1;
  RendererImpl::prepare_imgui();
  ImGui_ImplParanoixa_Init(&init_info);

  Shader vs;
  vs.load_default_vertex_shader();
  Shader vsInstanced;
  vsInstanced.load_default_vertex_instance_shader();
  Shader fs;
  fs.load_default_fragment_shader();

  this->pipeline3D.set_vertex_shader(vs);
  this->pipeline3D.set_vertex_instanced_shader(vsInstanced);
  this->pipeline3D.set_fragment_shader(fs);
  this->pipeline3D.build();
  this->currentPipeline3D = this->pipeline3D;

  this->pipeline2D.set_vertex_shader(vs);
  this->pipeline2D.set_fragment_shader(fs);
  this->pipeline2D.build();
  this->currentPipeline2D = this->pipeline2D;

  // Create depth stencil target
  {

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
  depthStencilInfo.cycle = 0;
  depthStencilInfo.stencilLoadOp = px::LoadOp::Clear;
  depthStencilInfo.stencilStoreOp = px::StoreOp::Store;
}
void PxRenderer::shutdown() {
  this->vertexArrays.clear();
  this->device.reset();
  this->backend.reset();
}
void PxRenderer::unload_data() {}
void PxRenderer::render() {
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
  if (WindowImpl::resized()) {
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
    this->depthStencilInfo.texture = depthTexture;
  }
  currentDepthStencilInfo = depthStencilInfo;

  ImGui_ImplParanoixa_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();
  for (auto &func : RendererImpl::get_imgui_function()) {
    func();
  }
  // Rendering
  ImGui::Render();
  ImDrawData *draw_data = ImGui::GetDrawData();

  Imgui_ImplParanoixa_PrepareDrawData(draw_data, commandBuffer);
  isFrameStarted = true;
  objectCount = 0;
  if (scene_system::is_run_script) {
    ScriptSystem::DrawScene();
  }
  if (objectCount > 0 && !isDraw2D) {
    commandBuffer->EndRenderPass(currentRenderPass);
  }

  {

    if (isFrameStarted || (objectCount > 0 && !isDraw2D)) {
      colorTargets[0].loadOp = px::LoadOp::Load;
      currentRenderPass = commandBuffer->BeginRenderPass(colorTargets, {});
    }
    auto renderPass = currentRenderPass;
    renderPass->SetViewport(
        px::Viewport{0, 0, Window::size().x, Window::size().y, 0, 1});
    renderPass->SetScissor(0, 0, Window::size().x, Window::size().y);
    // Render ImGui
    ImGui_ImplParanoixa_RenderDrawData(draw_data, commandBuffer, renderPass);
    commandBuffer->EndRenderPass(renderPass);
  }
  device->SubmitCommandBuffer(commandBuffer);
  device->WaitForGPUIdle();
}
void PxRenderer::draw2d(const std::shared_ptr<Drawable> draw_object) {
  objectCount++;
  if (isFrameStarted || currentRenderPass == nullptr) {
    colorTargets[0].loadOp = px::LoadOp::Clear;
    currentColorTargets = colorTargets;
    currentRenderPass =
        currentCommandBuffer->BeginRenderPass(currentColorTargets, {});
    auto renderPass = currentRenderPass;
    renderPass->SetViewport(
        px::Viewport{0, 0, Window::size().x, Window::size().y, 0, 1});
    renderPass->SetScissor(0, 0, Window::size().x, Window::size().y);
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
    renderPass->SetViewport(
        px::Viewport{0, 0, Window::size().x, Window::size().y, 0, 1});
    renderPass->SetScissor(0, 0, Window::size().x, Window::size().y);
    isDraw2D = true;
    isDefaultPipeline = true;
  }

  PxDrawable drawable{allocator};
  drawable.drawable = draw_object;

  auto texture = std::static_pointer_cast<px::Texture>(
      GetTexData(draw_object->binding_texture.textureData)->texture);
  drawable.textureSamplers.push_back(
      px::TextureSamplerBinding{.sampler = sampler, .texture = texture});

  auto modelData = GetModelData(draw_object->model.data);
  assert(modelData->vertexBuffer != nullptr);
  assert(modelData->indexBuffer != nullptr);

  drawable.vertexBuffers.emplace_back(
      px::BufferBinding{.buffer = modelData->vertexBuffer, .offset = 0});
  drawable.indexBuffer =
      px::BufferBinding{.buffer = modelData->indexBuffer, .offset = 0};

  auto commandBuffer = currentCommandBuffer;
  auto renderPass = currentRenderPass;
  renderPass->BindGraphicsPipeline(currentPipeline2D.get());
  renderPass->BindFragmentSamplers(0, drawable.textureSamplers);
  renderPass->BindVertexBuffers(0, drawable.vertexBuffers);
  renderPass->BindIndexBuffer(drawable.indexBuffer,
                              px::IndexElementSize::Uint32);

  auto param = drawable.drawable->param;
  commandBuffer->PushVertexUniformData(0, &param, sizeof(Drawable::parameter));
  renderPass->DrawIndexedPrimitives(
      GetModelData(drawable.drawable->model.data)->v_array.indexCount, 1, 0, 0,
      0);
}
void PxRenderer::draw3d(const std::shared_ptr<Drawable> draw_object) {
  objectCount++;
  if (isDefaultPipeline && (isFrameStarted || currentRenderPass == nullptr)) {
    colorTargets[0].loadOp = px::LoadOp::Clear;
    currentColorTargets = colorTargets;
    currentDepthStencilInfo = depthStencilInfo;
    currentRenderPass = currentCommandBuffer->BeginRenderPass(
        currentColorTargets, currentDepthStencilInfo);
    auto renderPass = currentRenderPass;
    renderPass->SetViewport(
        px::Viewport{0, 0, Window::size().x, Window::size().y, 0, 1});
    renderPass->SetScissor(0, 0, Window::size().x, Window::size().y);
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
    renderPass->SetViewport(
        px::Viewport{0, 0, Window::size().x, Window::size().y, 0, 1});
    renderPass->SetScissor(0, 0, Window::size().x, Window::size().y);
    isDraw2D = false;
    isDefaultPipeline = true;
  }
  PxDrawable drawable{allocator};
  drawable.drawable = draw_object;
  auto texture = std::static_pointer_cast<px::Texture>(
      GetTexData(draw_object->binding_texture.textureData)->texture);
  drawable.textureSamplers.push_back(
      px::TextureSamplerBinding{.sampler = sampler, .texture = texture});

  bool isInstance = drawable.drawable->size() > 0;
  if (isInstance) {
    px::Buffer::CreateInfo instanceBufferInfo{};
    instanceBufferInfo.allocator = allocator;
    instanceBufferInfo.size = drawable.drawable->size();
    instanceBufferInfo.usage = px::BufferUsage::Vertex;
    auto instanceBuffer = device->CreateBuffer(instanceBufferInfo);
    Ptr<px::TransferBuffer> transferBuffer;
    {
      px::TransferBuffer::CreateInfo info{};
      info.allocator = allocator;
      info.size = drawable.drawable->size();
      info.usage = px::TransferBufferUsage::Upload;
      transferBuffer = device->CreateTransferBuffer(info);
      auto *pMapped = transferBuffer->Map(false);
      if (pMapped) {

        memcpy(pMapped, drawable.drawable->data.data(),
               drawable.drawable->size());
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
          dst.size = drawable.drawable->size();
          dst.buffer = instanceBuffer;
          copyPass->UploadBuffer(src, dst, false);
        }
        commandBuffer->EndCopyPass(copyPass);
      }
      device->SubmitCommandBuffer(commandBuffer);
    }

    drawable.vertexBuffers.emplace_back(px::BufferBinding{
        .buffer = GetModelData(draw_object->model.data)->vertexBuffer,
        .offset = 0});
    drawable.indexBuffer = px::BufferBinding{
        .buffer = GetModelData(draw_object->model.data)->indexBuffer,
        .offset = 0};
    drawable.vertexBuffers.emplace_back(
        px::BufferBinding{.buffer = instanceBuffer, .offset = 0

        });
    auto commandBuffer = currentCommandBuffer;
    auto renderPass = currentRenderPass;
    renderPass->BindGraphicsPipeline(currentPipeline3D.get_instanced());
    renderPass->BindFragmentSamplers(0, drawable.textureSamplers);
    renderPass->BindVertexBuffers(0, drawable.vertexBuffers);
    renderPass->BindIndexBuffer(drawable.indexBuffer,
                                px::IndexElementSize::Uint32);

    auto param = drawable.drawable->param;
    commandBuffer->PushVertexUniformData(0, &param,
                                         sizeof(Drawable::parameter));
    renderPass->DrawIndexedPrimitives(
        GetModelData(drawable.drawable->model.data)->v_array.indexCount,
        drawable.drawable->data.size(), 0, 0, 0);
  } else {

    drawable.vertexBuffers.emplace_back(px::BufferBinding{
        .buffer = GetModelData(draw_object->model.data)->vertexBuffer,
        .offset = 0});
    drawable.indexBuffer = px::BufferBinding{
        .buffer = GetModelData(draw_object->model.data)->indexBuffer,
        .offset = 0};
    auto commandBuffer = currentCommandBuffer;
    auto renderPass = currentRenderPass;
    renderPass->BindGraphicsPipeline(currentPipeline3D.get());
    renderPass->BindFragmentSamplers(0, drawable.textureSamplers);
    renderPass->BindVertexBuffers(0, drawable.vertexBuffers);
    renderPass->BindIndexBuffer(drawable.indexBuffer,
                                px::IndexElementSize::Uint32);

    auto param = drawable.drawable->param;
    commandBuffer->PushVertexUniformData(0, &param,
                                         sizeof(Drawable::parameter));
    renderPass->DrawIndexedPrimitives(
        GetModelData(drawable.drawable->model.data)->v_array.indexCount, 1, 0,
        0, 0);
  }
}

void PxRenderer::load_shader(const Shader &shaderinfo) {}
void PxRenderer::unload_shader(const Shader &shaderinfo) {}
void PxRenderer::prepare_imgui() {}
void *PxRenderer::get_texture_id() { return nullptr; }
void PxRenderer::begin_pipeline3d(const RenderPipeline3D &pipeline) {
  currentPipeline3D = pipeline;
}
void PxRenderer::end_pipeline3d() { currentPipeline3D = pipeline3D; }
void PxRenderer::begin_pipeline2d(const RenderPipeline2D &pipeline) {
  currentPipeline2D = pipeline;
}
void PxRenderer::end_pipeline2d() { currentPipeline2D = pipeline2D; }
void PxRenderer::set_uniform_data(uint32_t slot, const UniformData &data) {
  currentCommandBuffer->PushVertexUniformData(slot, data.data.data(),
                                              data.data.size() * sizeof(float));
}
void PxRenderer::begin_render_texture2d(const RenderTexture &texture) {
  auto tex = texture.get_texture();
  auto depthTex = texture.get_depth_stencil();
  currentCommandBuffer = device->AcquireCommandBuffer({allocator});
  currentColorTargets[0].loadOp = px::LoadOp::Clear;
  currentColorTargets[0].texture = tex;
  currentRenderPass =
      currentCommandBuffer->BeginRenderPass(currentColorTargets, {});
  currentRenderPass->SetViewport(
      px::Viewport{0, 0, (float)texture.width, (float)texture.height, 0, 1});
  currentRenderPass->SetScissor(0, 0, (float)texture.width,
                                (float)texture.height);
  isDefaultPipeline = false;
}
void PxRenderer::begin_render_texture3d(const RenderTexture &texture) {
  auto tex = texture.get_texture();
  auto depthTex = texture.get_depth_stencil();
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
void PxRenderer::end_render_texture(const RenderTexture &texture,
                                    Texture &out) {
  currentCommandBuffer->EndRenderPass(currentRenderPass);
  device->SubmitCommandBuffer(currentCommandBuffer);
  device->WaitForGPUIdle();
  currentCommandBuffer = mainCommandBuffer;

  auto tex = texture.get_texture();
  auto outTextureData = GetTexData(out.textureData);

  // Copy
  px::TransferBuffer::CreateInfo info2{};
  info2.allocator = allocator;
  info2.size = texture.width * texture.height * 4;
  info2.usage = px::TransferBufferUsage::Download;
  auto transferBuffer = device->CreateTransferBuffer(info2);
  {
    px::CommandBuffer::CreateInfo info{};
    info.allocator = allocator;
    auto commandBuffer = device->AcquireCommandBuffer(info);
    {
      auto copyPass = commandBuffer->BeginCopyPass();
      {
        px::TextureLocation src{};
        src.texture = tex;
        src.layer = 0;
        src.mipLevel = 0;
        px::TextureLocation dst{};
        dst.texture = outTextureData->texture;
        dst.layer = 0;
        dst.mipLevel = 0;
        copyPass->CopyTexture(src, dst, texture.width, texture.height, 1,
                              false);
      }
      commandBuffer->EndCopyPass(copyPass);
    }
    device->SubmitCommandBuffer(commandBuffer);
    device->WaitForGPUIdle();
  }
  currentRenderPass = nullptr;
}
} // namespace sinen