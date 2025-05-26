#include "px_renderer.hpp"
#include "render_system.hpp"
#include <asset/model/vertex.hpp>
#include <cstddef>
#include <render/render_pipeline.hpp>

namespace sinen {

static px::VertexInputState CreateVertexInputState(px::Allocator *allocator,
                                                   bool isInstance,
                                                   bool isAnimation);
void RenderPipeline2D::set_vertex_shader(const Shader &shader) {
  this->vertexShader = shader;
}
void RenderPipeline2D::set_fragment_shader(const Shader &shader) {
  this->fragmentShader = shader;
}

void RenderPipeline2D::build() {
  auto pxRenderer = RendererImpl::GetPxRenderer();
  auto *allocator = pxRenderer->GetAllocator();
  auto device = pxRenderer->GetDevice();

  px::GraphicsPipeline::CreateInfo pipelineInfo{allocator};
  pipelineInfo.vertexShader = this->vertexShader.shader;
  pipelineInfo.fragmentShader = this->fragmentShader.shader;
  pipelineInfo.vertexInputState =
      CreateVertexInputState(allocator, false, false);
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
  this->pipeline = device->CreateGraphicsPipeline(pipelineInfo);
}
void RenderPipeline3D::set_vertex_shader(const Shader &shader) {
  this->vertexShader = shader;
}
void RenderPipeline3D::set_vertex_instanced_shader(const Shader &shader) {
  this->instancedShader = shader;
}
void RenderPipeline3D::set_fragment_shader(const Shader &shader) {
  this->fragmentShader = shader;
}
void RenderPipeline3D::set_animation(bool animation) {
  this->isAnimation = animation;
}
void RenderPipeline3D::build() {
  auto pxRenderer = RendererImpl::GetPxRenderer();
  auto *allocator = pxRenderer->GetAllocator();
  auto device = pxRenderer->GetDevice();

  px::GraphicsPipeline::CreateInfo pipelineInfo{allocator};
  pipelineInfo.vertexShader = this->vertexShader.shader;
  pipelineInfo.fragmentShader = this->fragmentShader.shader;
  pipelineInfo.vertexInputState =
      CreateVertexInputState(allocator, false, isAnimation);
  pipelineInfo.primitiveType = px::PrimitiveType::TriangleList;

  px::RasterizerState rasterizerState{};
  rasterizerState.fillMode = px::FillMode::Fill;
  rasterizerState.cullMode = px::CullMode::None;
  rasterizerState.frontFace = px::FrontFace::CounterClockwise;

  pipelineInfo.rasterizerState = rasterizerState;
  pipelineInfo.multiSampleState = {};
  pipelineInfo.multiSampleState.sampleCount = px::SampleCount::x1;
  pipelineInfo.depthStencilState.enableDepthTest = true;
  pipelineInfo.depthStencilState.enableDepthWrite = true;
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
  pipelineInfo.targetInfo.hasDepthStencilTarget = true;
  pipelineInfo.targetInfo.depthStencilTargetFormat =
      px::TextureFormat::D32_FLOAT_S8_UINT;
  this->pipeline = device->CreateGraphicsPipeline(pipelineInfo);
  if (instancedShader.shader == nullptr)
    return;
  pipelineInfo.vertexShader = this->instancedShader.shader;
  pipelineInfo.vertexInputState =
      CreateVertexInputState(allocator, true, isAnimation);
  this->instanced = device->CreateGraphicsPipeline(pipelineInfo);
}
px::VertexInputState CreateVertexInputState(px::Allocator *allocator,
                                            bool isInstance, bool isAnimation) {
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
    if (isAnimation) {
      vertexInputState.vertexAttributes.emplace_back(
          px::VertexAttribute{.location = 8,
                              .bufferSlot = 0,
                              .format = px::VertexElementFormat::Float4,
                              .offset = sizeof(Vertex)});
      vertexInputState.vertexAttributes.emplace_back(
          px::VertexAttribute{.location = 9,
                              .bufferSlot = 0,
                              .format = px::VertexElementFormat::Float4,
                              .offset = sizeof(Vertex) + sizeof(float) * 4});
    }
  } else {
    px::VertexBufferDescription desc{
        .slot = 0,
        .pitch = sizeof(Vertex),
        .inputRate = px::VertexInputRate::Vertex,
        .instanceStepRate = 0,
    };
    if (isAnimation) {
      desc.pitch = sizeof(AnimationVertex);
    }
    vertexInputState.vertexBufferDescriptions.emplace_back(desc);
    vertexInputState.vertexAttributes.emplace_back(
        px::VertexAttribute{.location = 0,
                            .bufferSlot = 0,
                            .format = px::VertexElementFormat::Float3,
                            .offset = offsetof(AnimationVertex, position)});
    vertexInputState.vertexAttributes.emplace_back(
        px::VertexAttribute{.location = 1,
                            .bufferSlot = 0,
                            .format = px::VertexElementFormat::Float3,
                            .offset = offsetof(AnimationVertex, normal)});
    vertexInputState.vertexAttributes.emplace_back(
        px::VertexAttribute{.location = 2,
                            .bufferSlot = 0,
                            .format = px::VertexElementFormat::Float2,
                            .offset = offsetof(AnimationVertex, uv)});
    vertexInputState.vertexAttributes.emplace_back(
        px::VertexAttribute{.location = 3,
                            .bufferSlot = 0,
                            .format = px::VertexElementFormat::Float4,
                            .offset = offsetof(AnimationVertex, rgba)});
    if (isAnimation) {
      vertexInputState.vertexAttributes.emplace_back(
          px::VertexAttribute{.location = 4,
                              .bufferSlot = 0,
                              .format = px::VertexElementFormat::Float4,
                              .offset = offsetof(AnimationVertex, boneIDs)});
      vertexInputState.vertexAttributes.emplace_back(px::VertexAttribute{
          .location = 5,
          .bufferSlot = 0,
          .format = px::VertexElementFormat::Float4,
          .offset = offsetof(AnimationVertex, boneWeights)});
    }
  }
  return vertexInputState;
}
} // namespace sinen