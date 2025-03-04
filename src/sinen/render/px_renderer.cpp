#include "px_renderer.hpp"

#include "../window/window_system.hpp"
#include "SDL3/SDL_video.h"
#include "drawable/instance_data.hpp"
#include "paranoixa/paranoixa.hpp"
#include "render/renderer.hpp"
#include "render_system.hpp"
#include <SDL3/SDL.h>
#include <cassert>
#include <io/data_stream.hpp>
#include <memory>
#include <window/window.hpp>

#include <imgui.h>
#include <imgui_impl_paranoixa.hpp>
#include <imgui_impl_sdl3.h>

#include "../model/model_data.hpp"
#include "../script/script_system.hpp"
#include "../texture/texture_data.hpp"
#include "script/script.hpp"
#include <sol/sol.hpp>

#include "../scene/scene_system.hpp"
// TODO:
// - Refactoring
// - Shadow mapping

namespace sinen {
PxDrawable::PxDrawable(px::Allocator *allocator)
    : allocator(allocator), vertexBuffers(allocator),
      textureSamplers(allocator) {}
px::VertexInputState CreateVertexInputState(px::Allocator *allocator,
                                            bool isInstance) {
  px::VertexInputState vertexInputState{allocator};
  if (isInstance) {
    vertexInputState.vertexBufferDescriptions.emplace_back(
        px::VertexBufferDescription{
            .slot = 0,
            .pitch = sizeof(Vertex),
            .inputRate = px::VertexInputRate::Vertex,
            .instanceStepRate = 0,
        });
    vertexInputState.vertexBufferDescriptions.emplace_back(
        px::VertexBufferDescription{
            .slot = 1,
            .pitch = sizeof(InstanceData),
            .inputRate = px::VertexInputRate::Instance,
            .instanceStepRate = 1,
        });
    vertexInputState.vertexAttributes.emplace_back(
        px::VertexAttribute{.location = 0,
                            .bufferSlot = 0,
                            .format = px::VertexElementFormat::Float3,
                            .offset = offsetof(Vertex, position)});
    vertexInputState.vertexAttributes.emplace_back(
        px::VertexAttribute{.location = 1,
                            .bufferSlot = 0,
                            .format = px::VertexElementFormat::Float3,
                            .offset = offsetof(Vertex, normal)});
    vertexInputState.vertexAttributes.emplace_back(
        px::VertexAttribute{.location = 2,
                            .bufferSlot = 0,
                            .format = px::VertexElementFormat::Float2,
                            .offset = offsetof(Vertex, uv)});
    vertexInputState.vertexAttributes.emplace_back(
        px::VertexAttribute{.location = 3,
                            .bufferSlot = 0,
                            .format = px::VertexElementFormat::Float4,
                            .offset = offsetof(Vertex, rgba)});

    vertexInputState.vertexAttributes.emplace_back(
        px::VertexAttribute{.location = 4,
                            .bufferSlot = 1,
                            .format = px::VertexElementFormat::Float4,
                            .offset = offsetof(InstanceData, world_matrix_1)});
    vertexInputState.vertexAttributes.emplace_back(
        px::VertexAttribute{.location = 5,
                            .bufferSlot = 1,
                            .format = px::VertexElementFormat::Float4,
                            .offset = offsetof(InstanceData, world_matrix_2)});
    vertexInputState.vertexAttributes.emplace_back(
        px::VertexAttribute{.location = 6,
                            .bufferSlot = 1,
                            .format = px::VertexElementFormat::Float4,
                            .offset = offsetof(InstanceData, world_matrix_3)});
    vertexInputState.vertexAttributes.emplace_back(
        px::VertexAttribute{.location = 7,
                            .bufferSlot = 1,
                            .format = px::VertexElementFormat::Float4,
                            .offset = offsetof(InstanceData, world_matrix_4)});
  } else {
    vertexInputState.vertexBufferDescriptions.emplace_back(
        px::VertexBufferDescription{
            .slot = 0,
            .pitch = sizeof(Vertex),
            .inputRate = px::VertexInputRate::Vertex,
            .instanceStepRate = 0,
        });
    vertexInputState.vertexAttributes.emplace_back(
        px::VertexAttribute{.location = 0,
                            .bufferSlot = 0,
                            .format = px::VertexElementFormat::Float3,
                            .offset = offsetof(Vertex, position)});
    vertexInputState.vertexAttributes.emplace_back(
        px::VertexAttribute{.location = 1,
                            .bufferSlot = 0,
                            .format = px::VertexElementFormat::Float3,
                            .offset = offsetof(Vertex, normal)});
    vertexInputState.vertexAttributes.emplace_back(
        px::VertexAttribute{.location = 2,
                            .bufferSlot = 0,
                            .format = px::VertexElementFormat::Float2,
                            .offset = offsetof(Vertex, uv)});
    vertexInputState.vertexAttributes.emplace_back(
        px::VertexAttribute{.location = 3,
                            .bufferSlot = 0,
                            .format = px::VertexElementFormat::Float4,
                            .offset = offsetof(Vertex, rgba)});
  }
  return vertexInputState;
}
PxRenderer::PxRenderer(px::Allocator *allocator)
    : allocator(allocator), vertexArrays(allocator), colorTargets(allocator) {}
void PxRenderer::initialize() {
  backend = px::Paranoixa::CreateBackend(allocator, px::GraphicsAPI::SDLGPU);
  px::Device::CreateInfo info{};
  info.allocator = allocator;
  info.debugMode = true;
  device = backend->CreateDevice(info);
  auto *window = WindowImpl::get_sdl_window();
  device->ClaimWindow(window);
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui_ImplSDL3_InitForSDLGPU(window);
  ImGui_ImplParanoixa_InitInfo init_info = {};
  init_info.Allocator = allocator;
  init_info.Device = device;
  init_info.ColorTargetFormat = px::TextureFormat::B8G8R8A8_UNORM;
  init_info.MSAASamples = px::SampleCount::x1;
  RendererImpl::prepare_imgui();
  ImGui_ImplParanoixa_Init(&init_info);

  std::string vsStr =
      DataStream::open_as_string(AssetType::Shader, "shader.vert.spv");
  std::string fsStr =
      DataStream::open_as_string(AssetType::Shader, "shaderAlpha.frag.spv");

  px::Shader::CreateInfo vsInfo{};
  vsInfo.allocator = allocator;
  vsInfo.size = vsStr.size();
  vsInfo.data = vsStr.data();
  vsInfo.entrypoint = "main";
  vsInfo.format = px::ShaderFormat::SPIRV;
  vsInfo.stage = px::ShaderStage::Vertex;
  vsInfo.numSamplers = 0;
  vsInfo.numStorageBuffers = 0;
  vsInfo.numStorageTextures = 0;
  vsInfo.numUniformBuffers = 1;
  auto vs = device->CreateShader(vsInfo);

  px::Shader::CreateInfo fsInfo{};
  fsInfo.allocator = allocator;
  fsInfo.size = fsStr.size();
  fsInfo.data = fsStr.data();
  fsInfo.entrypoint = "main";
  fsInfo.format = px::ShaderFormat::SPIRV;
  fsInfo.stage = px::ShaderStage::Fragment;
  fsInfo.numSamplers = 1;
  fsInfo.numStorageBuffers = 0;
  fsInfo.numStorageTextures = 0;
  fsInfo.numUniformBuffers = 0;
  auto fs = device->CreateShader(fsInfo);
  px::GraphicsPipeline::CreateInfo pipelineInfo{allocator};
  pipelineInfo.vertexShader = vs;
  pipelineInfo.fragmentShader = fs;
  pipelineInfo.vertexInputState = CreateVertexInputState(allocator, false);
  pipelineInfo.primitiveType = px::PrimitiveType::TriangleList;

  px::RasterizerState rasterizerState{};
  rasterizerState.fillMode = px::FillMode::Fill;
  rasterizerState.cullMode = px::CullMode::None;
  rasterizerState.frontFace = px::FrontFace::CounterClockwise;

  pipelineInfo.rasterizerState = rasterizerState;
  pipelineInfo.multiSampleState = {};
  pipelineInfo.multiSampleState.sampleCount = px::SampleCount::x1;
  pipelineInfo.depthStencilState.enableDepthTest = false;
  pipelineInfo.depthStencilState.enableDepthWrite = false;
  pipelineInfo.depthStencilState.enableStencilTest = false;
  pipelineInfo.depthStencilState.compareOp = px::CompareOp::LessOrEqual;

  pipelineInfo.targetInfo.colorTargetDescriptions.emplace_back(
      px::ColorTargetDescription{
          .format = device->GetSwapchainFormat(),
          .blendState =
              px::ColorTargetBlendState{
                  .srcColorBlendFactor = px::BlendFactor::SrcAlpha,
                  .dstColorBlendFactor = px::BlendFactor::OneMinusSrcAlpha,
                  .colorBlendOp = px::BlendOp::Add,
                  .srcAlphaBlendFactor = px::BlendFactor::One,
                  .dstAlphaBlendFactor = px::BlendFactor::OneMinusSrcAlpha,
                  .alphaBlendOp = px::BlendOp::Add,
                  .colorWriteMask =
                      px::ColorComponent::R | px::ColorComponent::G |
                      px::ColorComponent::B | px::ColorComponent::A,
                  .enableBlend = true,
              },
      });
  pipelineInfo.targetInfo.hasDepthStencilTarget = false;

  pipeline2D = device->CreateGraphicsPipeline(pipelineInfo);

  pipelineInfo.depthStencilState.enableDepthTest = true;
  pipelineInfo.depthStencilState.enableDepthWrite = true;
  pipelineInfo.depthStencilState.enableStencilTest = false;
  pipelineInfo.depthStencilState.compareOp = px::CompareOp::LessOrEqual;
  pipelineInfo.targetInfo.hasDepthStencilTarget = true;
  pipelineInfo.targetInfo.depthStencilTargetFormat =
      px::TextureFormat::D32_FLOAT_S8_UINT;
  pipeline3D = device->CreateGraphicsPipeline(pipelineInfo);

  vsStr =
      DataStream::open_as_string(AssetType::Shader, "shader_instance.vert.spv");

  vsInfo.allocator = allocator;
  vsInfo.size = vsStr.size();
  vsInfo.data = vsStr.data();
  vsInfo.entrypoint = "main";
  vsInfo.format = px::ShaderFormat::SPIRV;
  vsInfo.stage = px::ShaderStage::Vertex;
  vsInfo.numSamplers = 0;
  vsInfo.numStorageBuffers = 0;
  vsInfo.numStorageTextures = 0;
  vsInfo.numUniformBuffers = 1;
  vs = device->CreateShader(vsInfo);
  pipelineInfo.vertexInputState = CreateVertexInputState(allocator, true);
  pipelineInfo.vertexShader = vs;
  pipeline3DInstanced = device->CreateGraphicsPipeline(pipelineInfo);

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
  auto swapchainTexture = device->AcquireSwapchainTexture(commandBuffer);
  if (swapchainTexture == nullptr) {
    return;
  }
  colorTargets[0].texture = swapchainTexture;
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

  ImGui_ImplParanoixa_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();
  for (auto &func : RendererImpl::get_imgui_function()) {
    func();
  }
  // Rendering
  ImGui::Render();
  ImDrawData *draw_data = ImGui::GetDrawData();

  currentCommandBuffer = commandBuffer;
  Imgui_ImplParanoixa_PrepareDrawData(draw_data, commandBuffer);
  isFrameStarted = true;
  objectCount = 0;
  if (scene_system::is_run_script) {

    sol::state *lua = (sol::state *)script_system::get_state();
    (*lua)["Draw"]();
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
  if (isFrameStarted) {
    colorTargets[0].loadOp = px::LoadOp::Clear;
    currentRenderPass = currentCommandBuffer->BeginRenderPass(colorTargets, {});
    auto renderPass = currentRenderPass;
    renderPass->SetViewport(
        px::Viewport{0, 0, Window::size().x, Window::size().y, 0, 1});
    renderPass->SetScissor(0, 0, Window::size().x, Window::size().y);
    renderPass->BindGraphicsPipeline(pipeline2D);
    isFrameStarted = false;
    isDraw2D = true;
  } else if (!isDraw2D) {
    currentCommandBuffer->EndRenderPass(currentRenderPass);
    colorTargets[0].loadOp = px::LoadOp::Load;
    currentRenderPass = currentCommandBuffer->BeginRenderPass(colorTargets, {});
    auto renderPass = currentRenderPass;
    renderPass->SetViewport(
        px::Viewport{0, 0, Window::size().x, Window::size().y, 0, 1});
    renderPass->SetScissor(0, 0, Window::size().x, Window::size().y);
    renderPass->BindGraphicsPipeline(pipeline2D);
    isDraw2D = true;
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
  if (isFrameStarted) {
    colorTargets[0].loadOp = px::LoadOp::Clear;
    currentRenderPass =
        currentCommandBuffer->BeginRenderPass(colorTargets, depthStencilInfo);
    auto renderPass = currentRenderPass;
    renderPass->SetViewport(
        px::Viewport{0, 0, Window::size().x, Window::size().y, 0, 1});
    renderPass->SetScissor(0, 0, Window::size().x, Window::size().y);
    isFrameStarted = false;
    isDraw2D = false;
  } else if (isDraw2D) {
    currentCommandBuffer->EndRenderPass(currentRenderPass);
    colorTargets[0].loadOp = px::LoadOp::Load;
    currentRenderPass =
        currentCommandBuffer->BeginRenderPass(colorTargets, depthStencilInfo);
    auto renderPass = currentRenderPass;
    renderPass->SetViewport(
        px::Viewport{0, 0, Window::size().x, Window::size().y, 0, 1});
    renderPass->SetScissor(0, 0, Window::size().x, Window::size().y);
    isDraw2D = false;
  }
  PxDrawable drawable{allocator};
  drawable.drawable = draw_object;
  auto texture = std::static_pointer_cast<px::Texture>(
      GetTexData(draw_object->binding_texture.textureData)->texture);
  drawable.textureSamplers.push_back(
      px::TextureSamplerBinding{.sampler = sampler, .texture = texture});

  if (drawable.drawable->size() > 0) {
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
    renderPass->BindGraphicsPipeline(pipeline3DInstanced);
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
    renderPass->BindGraphicsPipeline(pipeline3D);
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
} // namespace sinen