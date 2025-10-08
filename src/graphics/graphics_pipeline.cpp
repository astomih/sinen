#include "graphics_system.hpp"

#include <cstddef>

#include <asset/model/vertex.hpp>
#include <core/logger/logger.hpp>
#include <graphics/graphics_pipeline.hpp>

namespace sinen {

static px::VertexInputState CreateVertexInputState(px::Allocator *allocator,
                                                   bool isInstance,
                                                   bool isAnimation);
void GraphicsPipeline2D::setVertexShader(const Shader &shader) {
  this->vertexShader = shader;
}
void GraphicsPipeline2D::setFragmentShader(const Shader &shader) {
  this->fragmentShader = shader;
}

void GraphicsPipeline2D::build() {
  auto *allocator = GraphicsSystem::getAllocator();
  auto device = GraphicsSystem::getDevice();

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
void GraphicsPipeline3D::setVertexShader(const Shader &shader) {
  this->vertexShader = shader;
}
void GraphicsPipeline3D::setFragmentShader(const Shader &shader) {
  this->fragmentShader = shader;
}
void GraphicsPipeline3D::setAnimation(bool animation) {
  this->isAnimation = animation;
}
void GraphicsPipeline3D::build() {
  auto *allocator = GraphicsSystem::getAllocator();
  auto device = GraphicsSystem::getDevice();

  px::GraphicsPipeline::CreateInfo pipelineInfo{allocator};
  pipelineInfo.vertexShader = this->vertexShader.shader;
  pipelineInfo.fragmentShader = this->fragmentShader.shader;
  pipelineInfo.vertexInputState =
      CreateVertexInputState(allocator, isInstanced, isAnimation);
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
}
px::VertexInputState CreateVertexInputState(px::Allocator *allocator,
                                            bool isInstance, bool isAnimation) {
  px::VertexInputState vertexInputState{allocator};
  uint32_t location = 0;
  uint32_t bufferSlot = 0;
  vertexInputState.vertexBufferDescriptions.emplace_back(
      px::VertexBufferDescription{
          .slot = bufferSlot,
          .pitch = sizeof(Vertex),
          .inputRate = px::VertexInputRate::Vertex,
          .instanceStepRate = 0,
      });
  vertexInputState.vertexAttributes.emplace_back(
      px::VertexAttribute{.location = location++,
                          .bufferSlot = bufferSlot,
                          .format = px::VertexElementFormat::Float3,
                          .offset = offsetof(Vertex, position)});
  vertexInputState.vertexAttributes.emplace_back(
      px::VertexAttribute{.location = location++,
                          .bufferSlot = bufferSlot,
                          .format = px::VertexElementFormat::Float3,
                          .offset = offsetof(Vertex, normal)});
  vertexInputState.vertexAttributes.emplace_back(
      px::VertexAttribute{.location = location++,
                          .bufferSlot = bufferSlot,
                          .format = px::VertexElementFormat::Float2,
                          .offset = offsetof(Vertex, uv)});
  vertexInputState.vertexAttributes.emplace_back(
      px::VertexAttribute{.location = location++,
                          .bufferSlot = bufferSlot,
                          .format = px::VertexElementFormat::Float4,
                          .offset = offsetof(Vertex, color)});
  if (isInstance) {
    bufferSlot++;
    vertexInputState.vertexBufferDescriptions.emplace_back(
        px::VertexBufferDescription{
            .slot = bufferSlot,
            .pitch = sizeof(InstanceData),
            .inputRate = px::VertexInputRate::Instance,
            .instanceStepRate = 0,
        });
    vertexInputState.vertexAttributes.emplace_back(
        px::VertexAttribute{.location = location++,
                            .bufferSlot = bufferSlot,
                            .format = px::VertexElementFormat::Float4,
                            .offset = offsetof(InstanceData, world_matrix_1)});
    vertexInputState.vertexAttributes.emplace_back(
        px::VertexAttribute{.location = location++,
                            .bufferSlot = bufferSlot,
                            .format = px::VertexElementFormat::Float4,
                            .offset = offsetof(InstanceData, world_matrix_2)});
    vertexInputState.vertexAttributes.emplace_back(
        px::VertexAttribute{.location = location++,
                            .bufferSlot = bufferSlot,
                            .format = px::VertexElementFormat::Float4,
                            .offset = offsetof(InstanceData, world_matrix_3)});
    vertexInputState.vertexAttributes.emplace_back(
        px::VertexAttribute{.location = location++,
                            .bufferSlot = bufferSlot,
                            .format = px::VertexElementFormat::Float4,
                            .offset = offsetof(InstanceData, world_matrix_4)});
  }
  if (isAnimation) {
    bufferSlot++;
    vertexInputState.vertexBufferDescriptions.emplace_back(
        px::VertexBufferDescription{
            .slot = bufferSlot,
            .pitch = sizeof(AnimationVertex),
            .inputRate = px::VertexInputRate::Vertex,
            .instanceStepRate = 0,
        });
    vertexInputState.vertexAttributes.emplace_back(
        px::VertexAttribute{.location = location++,
                            .bufferSlot = bufferSlot,
                            .format = px::VertexElementFormat::Float4,
                            .offset = offsetof(AnimationVertex, boneIDs)});
    vertexInputState.vertexAttributes.emplace_back(
        px::VertexAttribute{.location = location++,
                            .bufferSlot = bufferSlot,
                            .format = px::VertexElementFormat::Float4,
                            .offset = offsetof(AnimationVertex, boneWeights)});
  }
  return vertexInputState;
}
} // namespace sinen