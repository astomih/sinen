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
#include "../texture/texture_data.hpp"

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
    : allocator(allocator), drawables2D(allocator),
      drawables2DInstanced(allocator), drawables3DInstanced(allocator),
      drawables3D(allocator), vertexArrays(allocator) {}
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
  px::Texture::CreateInfo depthStencilInfo{};
  depthStencilInfo.allocator = allocator;
  depthStencilInfo.width = Window::size().x;
  depthStencilInfo.height = Window::size().y;
  depthStencilInfo.layerCountOrDepth = 1;
  depthStencilInfo.type = px::TextureType::Texture2D;
  depthStencilInfo.usage = px::TextureUsage::DepthStencilTarget;
  depthStencilInfo.format = px::TextureFormat::D32_FLOAT_S8_UINT;
  depthStencilInfo.numLevels = 1;
  depthStencilInfo.sampleCount = px::SampleCount::x1;
  depthTexture = device->CreateTexture(depthStencilInfo);

  // Default sampler
  px::Sampler::CreateInfo samplerInfo{};
  samplerInfo.allocator = allocator;
  samplerInfo.minFilter = px::Filter::Nearest;
  samplerInfo.magFilter = px::Filter::Nearest;
  samplerInfo.addressModeU = px::AddressMode::Repeat;
  samplerInfo.addressModeV = px::AddressMode::Repeat;
  samplerInfo.maxAnisotropy = 1.f;
  sampler = device->CreateSampler(samplerInfo);
}
void PxRenderer::shutdown() {
  this->drawables2D.clear();
  this->drawables2DInstanced.clear();
  this->drawables3D.clear();
  this->drawables3DInstanced.clear();
  this->vertexArrays.clear();
  this->device.reset();
  this->backend.reset();
}
void PxRenderer::unload_data() {}
void PxRenderer::render() {
  auto commandBuffer = device->AcquireCommandBuffer({allocator});
  px::Array<px::ColorTargetInfo> colorTargets{allocator};
  auto swapchainTexture = device->AcquireSwapchainTexture(commandBuffer);
  if (swapchainTexture == nullptr) {
    drawables3D.clear();
    drawables3DInstanced.clear();
    drawables2D.clear();
    return;
  }
  if (WindowImpl::resized()) {
    px::Texture::CreateInfo depthStencilInfo{};
    depthStencilInfo.allocator = allocator;
    depthStencilInfo.width = Window::size().x;
    depthStencilInfo.height = Window::size().y;
    depthStencilInfo.layerCountOrDepth = 1;
    depthStencilInfo.type = px::TextureType::Texture2D;
    depthStencilInfo.usage = px::TextureUsage::DepthStencilTarget;
    depthStencilInfo.format = px::TextureFormat::D32_FLOAT_S8_UINT;
    depthStencilInfo.numLevels = 1;
    depthStencilInfo.sampleCount = px::SampleCount::x1;
    depthTexture = device->CreateTexture(depthStencilInfo);
  }
  colorTargets.push_back(px::ColorTargetInfo{
      .texture = swapchainTexture,
      .loadOp = px::LoadOp::Clear,
      .storeOp = px::StoreOp::Store,
  });
  px::DepthStencilTargetInfo depthStencilInfo{};
  depthStencilInfo.texture = depthTexture;
  depthStencilInfo.loadOp = px::LoadOp::Clear;
  depthStencilInfo.storeOp = px::StoreOp::Store;
  depthStencilInfo.clearDepth = 1.0f;
  depthStencilInfo.clearStencil = 0;
  depthStencilInfo.cycle = 0;
  depthStencilInfo.stencilLoadOp = px::LoadOp::Clear;
  depthStencilInfo.stencilStoreOp = px::StoreOp::Store;

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
  {

    auto renderPass =
        commandBuffer->BeginRenderPass(colorTargets, depthStencilInfo);
    renderPass->SetViewport(
        px::Viewport{0, 0, Window::size().x, Window::size().y, 0, 1});
    renderPass->SetScissor(0, 0, Window::size().x, Window::size().y);
    renderPass->BindGraphicsPipeline(pipeline3D);
    for (auto &drawable : drawables3D) {
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
    renderPass->BindGraphicsPipeline(pipeline3DInstanced);
    for (auto &drawable : drawables3DInstanced) {
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
    }
    commandBuffer->EndRenderPass(renderPass);
  }

  {

    colorTargets[0].loadOp = px::LoadOp::Load;
    auto renderPass = commandBuffer->BeginRenderPass(colorTargets, {});
    renderPass->SetViewport(
        px::Viewport{0, 0, Window::size().x, Window::size().y, 0, 1});
    renderPass->SetScissor(0, 0, Window::size().x, Window::size().y);
    renderPass->BindGraphicsPipeline(pipeline2D);
    for (int i = 0; i < drawables2D.size(); i++) {
      renderPass->BindFragmentSamplers(0, drawables2D[i].textureSamplers);
      renderPass->BindVertexBuffers(0, drawables2D[i].vertexBuffers);
      renderPass->BindIndexBuffer(drawables2D[i].indexBuffer,
                                  px::IndexElementSize::Uint32);

      auto param = drawables2D[i].drawable->param;
      commandBuffer->PushVertexUniformData(0, &param,
                                           sizeof(Drawable::parameter));
      renderPass->DrawIndexedPrimitives(
          GetModelData(drawables2D[i].drawable->model.data)->v_array.indexCount,
          1, 0, 0, 0);
    }
    // Render ImGui
    ImGui_ImplParanoixa_RenderDrawData(draw_data, commandBuffer, renderPass);
    commandBuffer->EndRenderPass(renderPass);
  }
  device->SubmitCommandBuffer(commandBuffer);
  device->WaitForGPUIdle();
  drawables3D.clear();
  drawables3DInstanced.clear();
  drawables2D.clear();
}
void PxRenderer::draw2d(const std::shared_ptr<Drawable> draw_object) {

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

  drawables2D.push_back(drawable);
}
void PxRenderer::draw3d(const std::shared_ptr<Drawable> draw_object) {
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
    drawables3DInstanced.push_back(drawable);
  } else {

    drawable.vertexBuffers.emplace_back(px::BufferBinding{
        .buffer = GetModelData(draw_object->model.data)->vertexBuffer,
        .offset = 0});
    drawable.indexBuffer = px::BufferBinding{
        .buffer = GetModelData(draw_object->model.data)->indexBuffer,
        .offset = 0};
    drawables3D.push_back(drawable);
  }
}

void PxRenderer::add_vertex_array(const VertexArray &vArray,
                                  std::string_view name) {
  PxVertexArray vertexArray;
  vertexArray.indexCount = vArray.indexCount;
  vertexArray.indices = vArray.indices;
  vertexArray.vertices = vArray.vertices;
  vertexArray.materialName = vArray.materialName;
  auto vertexBufferSize = vArray.vertices.size() * sizeof(Vertex);

  px::Buffer::CreateInfo vertexBufferInfo{};
  vertexBufferInfo.allocator = allocator;
  vertexBufferInfo.size = vertexBufferSize;
  vertexBufferInfo.usage = px::BufferUsage::Vertex;
  vertexArray.vertexBuffer = device->CreateBuffer(vertexBufferInfo);

  px::Buffer::CreateInfo indexBufferInfo{};
  indexBufferInfo.allocator = allocator;
  indexBufferInfo.size = vArray.indices.size() * sizeof(uint32_t);
  indexBufferInfo.usage = px::BufferUsage::Index;
  vertexArray.indexBuffer = device->CreateBuffer(indexBufferInfo);

  Ptr<px::TransferBuffer> transferBuffer;
  {
    {
      px::TransferBuffer::CreateInfo info{};
      info.allocator = allocator;
      info.size = vertexBufferSize;
      info.usage = px::TransferBufferUsage::Upload;
      transferBuffer = device->CreateTransferBuffer(info);
      auto *pMapped = transferBuffer->Map(false);
      memcpy(pMapped, vArray.vertices.data(), vertexBufferSize);
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
          dst.size = vertexBufferSize;
          dst.buffer = vertexArray.vertexBuffer;
          copyPass->UploadBuffer(src, dst, false);
        }
        commandBuffer->EndCopyPass(copyPass);
      }
      device->SubmitCommandBuffer(commandBuffer);
    }
  }
  {
    px::TransferBuffer::CreateInfo info{};
    info.allocator = allocator;
    info.size = indexBufferInfo.size;
    info.usage = px::TransferBufferUsage::Upload;
    transferBuffer = device->CreateTransferBuffer(info);
    auto *pMapped = transferBuffer->Map(false);
    memcpy(pMapped, vArray.indices.data(), indexBufferInfo.size);
    transferBuffer->Unmap();
  }
  {
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
          dst.size = indexBufferInfo.size;
          dst.buffer = vertexArray.indexBuffer;
          copyPass->UploadBuffer(src, dst, false);
        }
        commandBuffer->EndCopyPass(copyPass);
      }
      device->SubmitCommandBuffer(commandBuffer);
    }
  }
  vertexArrays.insert(
      std::pair<std::string, PxVertexArray>(name.data(), vertexArray));
}
void PxRenderer::update_vertex_array(const VertexArray &vArray,
                                     std::string_view name) {}
void PxRenderer::add_model(const Model &m) {}
void PxRenderer::update_model(const Model &m) {}
void PxRenderer::load_shader(const Shader &shaderinfo) {}
void PxRenderer::unload_shader(const Shader &shaderinfo) {}
void PxRenderer::prepare_imgui() {}
void *PxRenderer::get_texture_id() { return nullptr; }
} // namespace sinen