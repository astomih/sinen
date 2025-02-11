#include "px_renderer.hpp"

#include "../window/window_system.hpp"
#include <SDL3/SDL.h>
#include <io/data_stream.hpp>
#include <window/window.hpp>

#include <imgui.h>
#include <imgui_impl_paranoixa.hpp>
#include <imgui_impl_sdl3.h>

namespace sinen {
PxDrawable::PxDrawable(px::AllocatorPtr allocator)
    : allocator(allocator), vertexBuffers(allocator),
      textureSamplers(allocator) {}
px::VertexInputState CreateVertexInputState(px::AllocatorPtr allocator,
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
PxRenderer::PxRenderer(px::AllocatorPtr allocator)
    : allocator(allocator), drawables2D(allocator), vertexArrays(allocator),
      textureSamplers(allocator) {}
void PxRenderer::initialize() {
  backend = px::Paranoixa::CreateBackend(allocator, px::GraphicsAPI::SDLGPU);
  px::Device::CreateInfo info{};
  info.allocator = allocator;
  info.debugMode = true;
  device = backend->CreateDevice(info);
  auto *window = WindowImpl::get_sdl_window();
  device->ClaimWindow(window);

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
}
void PxRenderer::shutdown() {}
void PxRenderer::unload_data() {}
void PxRenderer::render() {
  auto commandBuffer = device->CreateCommandBuffer({allocator});
  px::Array<px::ColorTargetInfo> colorTargets{allocator};
  auto swapchainTexture = device->AcquireSwapchainTexture(commandBuffer);
  colorTargets.push_back(px::ColorTargetInfo{
      .texture = swapchainTexture,
      .loadOp = px::LoadOp::Clear,
      .storeOp = px::StoreOp::Store,
  });
  auto renderPass = commandBuffer->BeginRenderPass(colorTargets);

  for (int i = 0; i < drawables2D.size(); i++) {
    renderPass->SetViewport(
        px::Viewport{0, 0, Window::size().x, Window::size().y, 0, 1});
    renderPass->BindGraphicsPipeline(pipeline2D);
    renderPass->BindFragmentSamplers(0, drawables2D[i].textureSamplers);
    Array<px::BufferBinding> vertexBuffers{allocator};
    vertexBuffers.push_back({vertexArrays["SPRITE"].vertexBuffer, 0});
    renderPass->BindVertexBuffers(0, vertexBuffers);
    renderPass->BindIndexBuffer({vertexArrays["SPRITE"].indexBuffer, 0},
                                px::IndexElementSize::Uint32);

    auto param = drawables2D[i].drawable.param;
    commandBuffer->PushVertexUniformData(0, &param,
                                         sizeof(Drawable::parameter));
    renderPass->DrawIndexedPrimitives(vertexArrays["SPRITE"].indexCount, 1, 0,
                                      0, 0);
  }
  commandBuffer->EndRenderPass(renderPass);
  device->SubmitCommandBuffer(commandBuffer);
  drawables2D.clear();
}
Ptr<px::Texture> PxRenderer::CreateNativeTexture(const HandleT &handle) {
  SDL_Surface *pSurface = reinterpret_cast<SDL_Surface *>(handle);
  SDL_Surface &surface = *pSurface;
  ::SDL_LockSurface(&surface);
  auto *pFormat = ::SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA8888);
  auto *pImageDataSurface = ::SDL_ConvertSurface(&surface, pFormat->format);
  ::SDL_UnlockSurface(&surface);

  Ptr<px::TransferBuffer> transferBuffer;
  int width = pImageDataSurface->w, height = pImageDataSurface->h;
  {
    px::TransferBuffer::CreateInfo info{};
    info.allocator = allocator;
    info.size = width * height * 4;
    info.usage = px::TransferBufferUsage::Upload;
    transferBuffer = device->CreateTransferBuffer(info);
    auto *pMapped = transferBuffer->Map(false);
    auto *pImage = pImageDataSurface->pixels;
    memcpy(pMapped, pImage, info.size);
    transferBuffer->Unmap();
  }
  Ptr<px::Texture> texture;
  {
    px::Texture::CreateInfo info{};
    info.allocator = allocator;
    info.width = width;
    info.height = height;
    info.layerCountOrDepth = 1;
    info.format = px::TextureFormat::R8G8B8A8_UNORM;
    info.usage = px::TextureUsage::Sampler;
    info.numLevels = 1;
    info.sampleCount = px::SampleCount::x1;
    info.type = px::TextureType::Texture2D;
    texture = device->CreateTexture(info);
  }
  {
    px::CommandBuffer::CreateInfo info{};
    info.allocator = allocator;
    auto commandBuffer = device->CreateCommandBuffer(info);
    auto copyPass = commandBuffer->BeginCopyPass();
    px::TextureTransferInfo src{};
    src.offset = 0;
    src.transferBuffer = transferBuffer;
    px::TextureRegion dst{};
    dst.x = 0;
    dst.y = 0;
    dst.width = width;
    dst.height = height;
    dst.depth = 1;
    dst.texture = texture;
    copyPass->UploadTexture(src, dst, false);
    commandBuffer->EndCopyPass(copyPass);
    device->SubmitCommandBuffer(commandBuffer);
  }
  return texture;
}
void PxRenderer::draw2d(const std::shared_ptr<Drawable> draw_object) {

  PxDrawable drawable{allocator};
  auto texture = CreateNativeTexture(draw_object->binding_texture.handle);
  px::Sampler::CreateInfo samplerInfo{};
  samplerInfo.allocator = allocator;
  samplerInfo.minFilter = px::Filter::Nearest;
  samplerInfo.magFilter = px::Filter::Nearest;
  samplerInfo.addressModeU = px::AddressMode::Repeat;
  samplerInfo.addressModeV = px::AddressMode::Repeat;
  samplerInfo.maxAnisotropy = 1.f;
  auto sampler = device->CreateSampler(samplerInfo);

  drawable.vertexBuffers.emplace_back(px::BufferBinding{
      .buffer = vertexArrays[draw_object->vertexIndex].vertexBuffer,
      .offset = 0});
  drawable.indexBuffer = px::BufferBinding{
      .buffer = vertexArrays[draw_object->vertexIndex].indexBuffer,
      .offset = 0};

  drawable.textureSamplers.emplace_back(
      px::TextureSamplerBinding{.sampler = sampler, .texture = texture});
}
void PxRenderer::drawui(const std::shared_ptr<Drawable> draw_object) {
  PxDrawable drawable{allocator};
  drawable.drawable = *draw_object;
  if (this->textureSamplers.find(draw_object->binding_texture.handle) ==
      this->textureSamplers.end()) {
    auto texture = CreateNativeTexture(draw_object->binding_texture.handle);
    px::Sampler::CreateInfo samplerInfo{};
    samplerInfo.allocator = allocator;
    samplerInfo.minFilter = px::Filter::Nearest;
    samplerInfo.magFilter = px::Filter::Nearest;
    samplerInfo.addressModeU = px::AddressMode::Repeat;
    samplerInfo.addressModeV = px::AddressMode::Repeat;
    samplerInfo.maxAnisotropy = 1.f;
    auto sampler = device->CreateSampler(samplerInfo);
    textureSamplers.insert(std::pair<HandleT, px::TextureSamplerBinding>(
        draw_object->binding_texture.handle,
        px::TextureSamplerBinding{.sampler = sampler, .texture = texture}));

    drawable.textureSamplers.push_back(
        textureSamplers[draw_object->binding_texture.handle]);
  } else {
    drawable.textureSamplers.push_back(
        textureSamplers[draw_object->binding_texture.handle]);
  }

  drawable.vertexBuffers.emplace_back(px::BufferBinding{
      .buffer = vertexArrays[draw_object->vertexIndex].vertexBuffer,
      .offset = 0});
  drawable.indexBuffer = px::BufferBinding{
      .buffer = vertexArrays[draw_object->vertexIndex].indexBuffer,
      .offset = 0};

  drawables2D.push_back(drawable);
}
void PxRenderer::draw3d(const std::shared_ptr<Drawable> draw_object) {}
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
      auto commandBuffer = device->CreateCommandBuffer(info);
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
      auto commandBuffer = device->CreateCommandBuffer(info);
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
void PxRenderer::add_model(const Model &m) {
  PxVertexArray vertexArray;
  vertexArray.indexCount = m.all_indices().size();
  vertexArray.indices = m.all_indices();
  vertexArray.vertices = m.all_vertex();
  vertexArray.materialName = m.v_array.materialName;
  auto vertexBufferSize = m.all_vertex().size() * sizeof(Vertex);

  px::Buffer::CreateInfo vertexBufferInfo{};
  vertexBufferInfo.allocator = allocator;
  vertexBufferInfo.size = vertexBufferSize;
  vertexBufferInfo.usage = px::BufferUsage::Vertex;
  vertexArray.vertexBuffer = device->CreateBuffer(vertexBufferInfo);

  px::Buffer::CreateInfo indexBufferInfo{};
  indexBufferInfo.allocator = allocator;
  indexBufferInfo.size = m.all_indices().size() * sizeof(uint32_t);
  indexBufferInfo.usage = px::BufferUsage::Index;
  vertexArray.indexBuffer = device->CreateBuffer(indexBufferInfo);

  Ptr<px::TransferBuffer> transferBuffer;
  {
    px::TransferBuffer::CreateInfo info{};
    info.allocator = allocator;
    info.size = vertexBufferSize;
    info.usage = px::TransferBufferUsage::Upload;
    transferBuffer = device->CreateTransferBuffer(info);
    auto *pMapped = transferBuffer->Map(false);
    memcpy(pMapped, m.all_vertex().data(), vertexBufferSize);
    transferBuffer->Unmap();
  }
  {
    px::CommandBuffer::CreateInfo info{};
    info.allocator = allocator;
    auto commandBuffer = device->CreateCommandBuffer(info);
    auto copyPass = commandBuffer->BeginCopyPass();
    px::BufferTransferInfo src{};
    src.offset = 0;
    src.transferBuffer = transferBuffer;
    px::BufferRegion dst{};
    dst.offset = 0;
    dst.size = vertexBufferSize;
    dst.buffer = vertexArray.vertexBuffer;
    copyPass->UploadBuffer(src, dst, false);
    commandBuffer->EndCopyPass(copyPass);
    device->SubmitCommandBuffer(commandBuffer);
  }
  vertexArrays.insert(
      std::pair<std::string, PxVertexArray>(m.name.data(), vertexArray));
}
void PxRenderer::update_model(const Model &m) {}
void PxRenderer::load_shader(const Shader &shaderinfo) {}
void PxRenderer::unload_shader(const Shader &shaderinfo) {}
void PxRenderer::prepare_imgui() {}
void *PxRenderer::get_texture_id() { return nullptr; }
} // namespace sinen