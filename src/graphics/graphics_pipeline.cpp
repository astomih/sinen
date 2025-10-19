#include "graphics_system.hpp"
#include "libs/SDL/src/video/khronos/vulkan/vulkan_core.h"

#include <cstddef>

#include <asset/model/vertex.hpp>
#include <core/logger/logger.hpp>
#include <graphics/graphics_pipeline.hpp>

namespace sinen {

static px::VertexInputState
CreateVertexInputState(px::Allocator *allocator, std::bitset<32> featureFlags);
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
  auto *allocator = GraphicsSystem::getAllocator();
  auto device = GraphicsSystem::getDevice();

  px::GraphicsPipeline::CreateInfo pipelineInfo{allocator};
  pipelineInfo.vertexShader = this->vertexShader.shader;
  pipelineInfo.fragmentShader = this->fragmentShader.shader;
  pipelineInfo.vertexInputState =
      CreateVertexInputState(allocator, featureFlags);
  pipelineInfo.primitiveType = px::PrimitiveType::TriangleList;

  px::RasterizerState rasterizerState{};
  rasterizerState.fillMode = px::FillMode::Fill;
  rasterizerState.cullMode = px::CullMode::None;
  rasterizerState.frontFace = px::FrontFace::CounterClockwise;

  pipelineInfo.rasterizerState = rasterizerState;
  pipelineInfo.multiSampleState = {};
  pipelineInfo.multiSampleState.sampleCount = px::SampleCount::x1;
  bool enableDepthTest =
      featureFlags.test(GraphicsPipeline::FeatureFlag::DepthTest);
  pipelineInfo.depthStencilState.enableDepthTest = enableDepthTest;
  pipelineInfo.depthStencilState.enableDepthWrite = enableDepthTest;
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
  pipelineInfo.targetInfo.hasDepthStencilTarget = enableDepthTest;
  pipelineInfo.targetInfo.depthStencilTargetFormat =
      px::TextureFormat::D32_FLOAT_S8_UINT;
  this->pipeline = device->CreateGraphicsPipeline(pipelineInfo);
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

px::VertexInputState CreateVertexInputState(px::Allocator *allocator,
                                            std::bitset<32> featureFlags) {
  bool isInstance = featureFlags.test(GraphicsPipeline::FeatureFlag::Instanced);
  bool isAnimation =
      featureFlags.test(GraphicsPipeline::FeatureFlag::Animation);
  bool isTangent = featureFlags.test(GraphicsPipeline::FeatureFlag::Tangent);
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
            .pitch = sizeof(glm::mat4),
            .inputRate = px::VertexInputRate::Instance,
            .instanceStepRate = 0,
        });
    uint32_t offset = 0;
    vertexInputState.vertexAttributes.emplace_back(
        px::VertexAttribute{.location = location++,
                            .bufferSlot = bufferSlot,
                            .format = px::VertexElementFormat::Float4,
                            .offset = offset});
    offset += sizeof(float) * 4;
    vertexInputState.vertexAttributes.emplace_back(
        px::VertexAttribute{.location = location++,
                            .bufferSlot = bufferSlot,
                            .format = px::VertexElementFormat::Float4,
                            .offset = offset});
    offset += sizeof(float) * 4;
    vertexInputState.vertexAttributes.emplace_back(
        px::VertexAttribute{.location = location++,
                            .bufferSlot = bufferSlot,
                            .format = px::VertexElementFormat::Float4,
                            .offset = offset});
    offset += sizeof(float) * 4;
    vertexInputState.vertexAttributes.emplace_back(
        px::VertexAttribute{.location = location++,
                            .bufferSlot = bufferSlot,
                            .format = px::VertexElementFormat::Float4,
                            .offset = offset});
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
  if (isTangent) {
    bufferSlot++;
    vertexInputState.vertexBufferDescriptions.emplace_back(
        px::VertexBufferDescription{
            .slot = bufferSlot,
            .pitch = sizeof(glm::vec4),
            .inputRate = px::VertexInputRate::Vertex,
            .instanceStepRate = 0,
        });
    vertexInputState.vertexAttributes.emplace_back(
        px::VertexAttribute{.location = location++,
                            .bufferSlot = bufferSlot,
                            .format = px::VertexElementFormat::Float4,
                            .offset = 0});
  }
  return vertexInputState;
}
} // namespace sinen