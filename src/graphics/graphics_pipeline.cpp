#include "core/allocator/global_allocator.hpp"
#include "graphics_system.hpp"

#include <cstddef>

#include <core/logger/logger.hpp>
#include <graphics/graphics_pipeline.hpp>

namespace sinen {

static rhi::VertexInputState
CreateVertexInputState(Allocator *allocator, std::bitset<32> featureFlags);
void GraphicsPipeline::setVertexShader(const Shader &shader) {
  this->vertexShader = shader;
}
void GraphicsPipeline::setFragmentShader(const Shader &shader) {
  this->fragmentShader = shader;
}
void GraphicsPipeline::setEnableDepthTest(bool enable) {
  featureFlags.set(DepthTest, enable);
}
void GraphicsPipeline::setEnableInstanced(bool enable) {
  featureFlags.set(Instanced, enable);
}
void GraphicsPipeline::setEnableAnimation(bool enable) {
  featureFlags.set(Animation, enable);
}
void GraphicsPipeline::setEnableTangent(bool enable) {
  featureFlags.set(Tangent, enable);
}
void GraphicsPipeline::build() {
  auto *allocator = GlobalAllocator::get();
  auto device = GraphicsSystem::getDevice();

  rhi::GraphicsPipeline::CreateInfo pipelineInfo{allocator};
  pipelineInfo.vertexShader = this->vertexShader.shader;
  pipelineInfo.fragmentShader = this->fragmentShader.shader;
  pipelineInfo.vertexInputState =
      CreateVertexInputState(allocator, featureFlags);
  pipelineInfo.primitiveType = rhi::PrimitiveType::TriangleList;

  rhi::RasterizerState rasterizerState{};
  rasterizerState.fillMode = rhi::FillMode::Fill;
  rasterizerState.cullMode = rhi::CullMode::None;
  rasterizerState.frontFace = rhi::FrontFace::CounterClockwise;

  pipelineInfo.rasterizerState = rasterizerState;
  pipelineInfo.multiSampleState = {};
  pipelineInfo.multiSampleState.sampleCount = rhi::SampleCount::x1;
  bool enableDepthTest =
      featureFlags.test(GraphicsPipeline::FeatureFlag::DepthTest);
  pipelineInfo.depthStencilState.enableDepthTest = enableDepthTest;
  pipelineInfo.depthStencilState.enableDepthWrite = enableDepthTest;
  pipelineInfo.depthStencilState.enableStencilTest = false;
  pipelineInfo.depthStencilState.compareOp = rhi::CompareOp::LessOrEqual;

  pipelineInfo.targetInfo.colorTargetDescriptions.emplace_back(
      rhi::ColorTargetDescription{
          .format = device->getSwapchainFormat(),
          .blendState =
              rhi::ColorTargetBlendState{
                  .srcColorBlendFactor = rhi::BlendFactor::SrcAlpha,
                  .dstColorBlendFactor = rhi::BlendFactor::OneMinusSrcAlpha,
                  .colorBlendOp = rhi::BlendOp::Add,
                  .srcAlphaBlendFactor = rhi::BlendFactor::One,
                  .dstAlphaBlendFactor = rhi::BlendFactor::OneMinusSrcAlpha,
                  .alphaBlendOp = rhi::BlendOp::Add,
                  .colorWriteMask =
                      rhi::ColorComponent::R | rhi::ColorComponent::G |
                      rhi::ColorComponent::B | rhi::ColorComponent::A,
                  .enableBlend = true,
              },
      });
  pipelineInfo.targetInfo.hasDepthStencilTarget = enableDepthTest;
  pipelineInfo.targetInfo.depthStencilTargetFormat =
      rhi::TextureFormat::D32_FLOAT_S8_UINT;
  this->pipeline = device->createGraphicsPipeline(pipelineInfo);
}

GraphicsPipeline BuiltinPipelines::get3D() {
  return GraphicsSystem::pipeline3D;
}
GraphicsPipeline BuiltinPipelines::get3DInstanced() {
  return GraphicsSystem::pipelineInstanced3D;
}
GraphicsPipeline BuiltinPipelines::get2D() {
  return GraphicsSystem::pipeline2D;
}

rhi::VertexInputState CreateVertexInputState(Allocator *allocator,
                                             std::bitset<32> featureFlags) {
  bool isInstance = featureFlags.test(GraphicsPipeline::FeatureFlag::Instanced);
  bool isAnimation =
      featureFlags.test(GraphicsPipeline::FeatureFlag::Animation);
  bool isTangent = featureFlags.test(GraphicsPipeline::FeatureFlag::Tangent);
  rhi::VertexInputState vertexInputState{allocator};
  uint32_t location = 0;
  uint32_t bufferSlot = 0;
  vertexInputState.vertexBufferDescriptions.emplace_back(
      rhi::VertexBufferDescription{
          .slot = bufferSlot,
          .pitch = sizeof(Vertex),
          .inputRate = rhi::VertexInputRate::Vertex,
          .instanceStepRate = 0,
      });
  vertexInputState.vertexAttributes.emplace_back(
      rhi::VertexAttribute{.location = location++,
                           .bufferSlot = bufferSlot,
                           .format = rhi::VertexElementFormat::Float3,
                           .offset = offsetof(Vertex, position)});
  vertexInputState.vertexAttributes.emplace_back(
      rhi::VertexAttribute{.location = location++,
                           .bufferSlot = bufferSlot,
                           .format = rhi::VertexElementFormat::Float3,
                           .offset = offsetof(Vertex, normal)});
  vertexInputState.vertexAttributes.emplace_back(
      rhi::VertexAttribute{.location = location++,
                           .bufferSlot = bufferSlot,
                           .format = rhi::VertexElementFormat::Float2,
                           .offset = offsetof(Vertex, uv)});
  vertexInputState.vertexAttributes.emplace_back(
      rhi::VertexAttribute{.location = location++,
                           .bufferSlot = bufferSlot,
                           .format = rhi::VertexElementFormat::Float4,
                           .offset = offsetof(Vertex, color)});
  if (isInstance) {
    bufferSlot++;
    vertexInputState.vertexBufferDescriptions.emplace_back(
        rhi::VertexBufferDescription{
            .slot = bufferSlot,
            .pitch = sizeof(glm::mat4),
            .inputRate = rhi::VertexInputRate::Instance,
            .instanceStepRate = 0,
        });
    uint32_t offset = 0;
    vertexInputState.vertexAttributes.emplace_back(
        rhi::VertexAttribute{.location = location++,
                             .bufferSlot = bufferSlot,
                             .format = rhi::VertexElementFormat::Float4,
                             .offset = offset});
    offset += sizeof(float) * 4;
    vertexInputState.vertexAttributes.emplace_back(
        rhi::VertexAttribute{.location = location++,
                             .bufferSlot = bufferSlot,
                             .format = rhi::VertexElementFormat::Float4,
                             .offset = offset});
    offset += sizeof(float) * 4;
    vertexInputState.vertexAttributes.emplace_back(
        rhi::VertexAttribute{.location = location++,
                             .bufferSlot = bufferSlot,
                             .format = rhi::VertexElementFormat::Float4,
                             .offset = offset});
    offset += sizeof(float) * 4;
    vertexInputState.vertexAttributes.emplace_back(
        rhi::VertexAttribute{.location = location++,
                             .bufferSlot = bufferSlot,
                             .format = rhi::VertexElementFormat::Float4,
                             .offset = offset});
  }
  if (isAnimation) {
    bufferSlot++;
    vertexInputState.vertexBufferDescriptions.emplace_back(
        rhi::VertexBufferDescription{
            .slot = bufferSlot,
            .pitch = sizeof(AnimationVertex),
            .inputRate = rhi::VertexInputRate::Vertex,
            .instanceStepRate = 0,
        });
    vertexInputState.vertexAttributes.emplace_back(
        rhi::VertexAttribute{.location = location++,
                             .bufferSlot = bufferSlot,
                             .format = rhi::VertexElementFormat::Float4,
                             .offset = offsetof(AnimationVertex, boneIDs)});
    vertexInputState.vertexAttributes.emplace_back(
        rhi::VertexAttribute{.location = location++,
                             .bufferSlot = bufferSlot,
                             .format = rhi::VertexElementFormat::Float4,
                             .offset = offsetof(AnimationVertex, boneWeights)});
  }
  if (isTangent) {
    bufferSlot++;
    vertexInputState.vertexBufferDescriptions.emplace_back(
        rhi::VertexBufferDescription{
            .slot = bufferSlot,
            .pitch = sizeof(glm::vec4),
            .inputRate = rhi::VertexInputRate::Vertex,
            .instanceStepRate = 0,
        });
    vertexInputState.vertexAttributes.emplace_back(
        rhi::VertexAttribute{.location = location++,
                             .bufferSlot = bufferSlot,
                             .format = rhi::VertexElementFormat::Float4,
                             .offset = 0});
  }
  return vertexInputState;
}
} // namespace sinen