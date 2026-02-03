#include "graphics.hpp"
#include <core/allocator/global_allocator.hpp>
#include <core/thread/load_context.hpp>
#include <graphics/graphics_pipeline.hpp>
#include <script/luaapi.hpp>

#include <cstddef>
#include <chrono>
#include <functional>
#include <memory>
namespace sinen {

static gpu::VertexInputState
createVertexInputState(Allocator *allocator, std::bitset<32> featureFlags);
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
  const TaskGroup group = LoadContext::current();
  group.add();

  const bool inSetup = (LoadContext::currentPtr() != nullptr);

  auto buildOrRetry = std::make_shared<std::function<void()>>();
  *buildOrRetry = [this, buildOrRetry, group]() {
    if (this->pipeline) {
      group.done();
      return;
    }

    if (!this->vertexShader.getRaw() || !this->fragmentShader.getRaw()) {
      Graphics::addPreDrawFunc(*buildOrRetry);
      return;
    }

    auto *allocator = GlobalAllocator::get();
    auto device = Graphics::getDevice();

    gpu::GraphicsPipeline::CreateInfo pipelineInfo{allocator};
    pipelineInfo.vertexShader = this->vertexShader.getRaw();
    pipelineInfo.fragmentShader = this->fragmentShader.getRaw();
    pipelineInfo.vertexInputState =
        createVertexInputState(allocator, featureFlags);
    pipelineInfo.primitiveType = gpu::PrimitiveType::TriangleList;

    gpu::RasterizerState rasterizerState{};
    rasterizerState.fillMode = gpu::FillMode::Fill;
    rasterizerState.cullMode = gpu::CullMode::None;
    rasterizerState.frontFace = gpu::FrontFace::CounterClockwise;

    pipelineInfo.rasterizerState = rasterizerState;
    pipelineInfo.multiSampleState = {};
    pipelineInfo.multiSampleState.sampleCount = gpu::SampleCount::x1;
    bool enableDepthTest =
        featureFlags.test(GraphicsPipeline::FeatureFlag::DepthTest);
    pipelineInfo.depthStencilState.enableDepthTest = enableDepthTest;
    pipelineInfo.depthStencilState.enableDepthWrite = enableDepthTest;
    pipelineInfo.depthStencilState.enableStencilTest = false;
    pipelineInfo.depthStencilState.compareOp = gpu::CompareOp::LessOrEqual;

    pipelineInfo.targetInfo.colorTargetDescriptions.emplace_back(
        gpu::ColorTargetDescription{
            .format = device->getSwapchainFormat(),
            .blendState =
                gpu::ColorTargetBlendState{
                    .srcColorBlendFactor = gpu::BlendFactor::SrcAlpha,
                    .dstColorBlendFactor = gpu::BlendFactor::OneMinusSrcAlpha,
                    .colorBlendOp = gpu::BlendOp::Add,
                    .srcAlphaBlendFactor = gpu::BlendFactor::One,
                    .dstAlphaBlendFactor = gpu::BlendFactor::OneMinusSrcAlpha,
                    .alphaBlendOp = gpu::BlendOp::Add,
                    .colorWriteMask =
                        gpu::ColorComponent::R | gpu::ColorComponent::G |
                        gpu::ColorComponent::B | gpu::ColorComponent::A,
                    .enableBlend = true,
                },
        });
    pipelineInfo.targetInfo.hasDepthStencilTarget = enableDepthTest;
    pipelineInfo.targetInfo.depthStencilTargetFormat =
        gpu::TextureFormat::D32_FLOAT_S8_UINT;
    this->pipeline = device->createGraphicsPipeline(pipelineInfo);
    group.done();
  };

  if (!inSetup && this->vertexShader.getRaw() && this->fragmentShader.getRaw()) {
    (*buildOrRetry)();
    return;
  }

  Graphics::addPreDrawFunc(*buildOrRetry);
}

gpu::VertexInputState createVertexInputState(Allocator *allocator,
                                             std::bitset<32> featureFlags) {
  bool isInstance = featureFlags.test(GraphicsPipeline::FeatureFlag::Instanced);
  bool isAnimation =
      featureFlags.test(GraphicsPipeline::FeatureFlag::Animation);
  bool isTangent = featureFlags.test(GraphicsPipeline::FeatureFlag::Tangent);
  gpu::VertexInputState vertexInputState{allocator};
  uint32_t location = 0;
  uint32_t bufferSlot = 0;
  vertexInputState.vertexBufferDescriptions.emplace_back(
      gpu::VertexBufferDescription{
          .slot = bufferSlot,
          .pitch = sizeof(Vertex),
          .inputRate = gpu::VertexInputRate::Vertex,
          .instanceStepRate = 0,
      });
  vertexInputState.vertexAttributes.emplace_back(
      gpu::VertexAttribute{.location = location++,
                           .bufferSlot = bufferSlot,
                           .format = gpu::VertexElementFormat::Float3,
                           .offset = offsetof(Vertex, position)});
  vertexInputState.vertexAttributes.emplace_back(
      gpu::VertexAttribute{.location = location++,
                           .bufferSlot = bufferSlot,
                           .format = gpu::VertexElementFormat::Float3,
                           .offset = offsetof(Vertex, normal)});
  vertexInputState.vertexAttributes.emplace_back(
      gpu::VertexAttribute{.location = location++,
                           .bufferSlot = bufferSlot,
                           .format = gpu::VertexElementFormat::Float2,
                           .offset = offsetof(Vertex, uv)});
  vertexInputState.vertexAttributes.emplace_back(
      gpu::VertexAttribute{.location = location++,
                           .bufferSlot = bufferSlot,
                           .format = gpu::VertexElementFormat::Float4,
                           .offset = offsetof(Vertex, color)});
  if (isInstance) {
    bufferSlot++;
    vertexInputState.vertexBufferDescriptions.emplace_back(
        gpu::VertexBufferDescription{
            .slot = bufferSlot,
            .pitch = sizeof(Mat4),
            .inputRate = gpu::VertexInputRate::Instance,
            .instanceStepRate = 0,
        });
    uint32_t offset = 0;
    vertexInputState.vertexAttributes.emplace_back(
        gpu::VertexAttribute{.location = location++,
                             .bufferSlot = bufferSlot,
                             .format = gpu::VertexElementFormat::Float4,
                             .offset = offset});
    offset += sizeof(float) * 4;
    vertexInputState.vertexAttributes.emplace_back(
        gpu::VertexAttribute{.location = location++,
                             .bufferSlot = bufferSlot,
                             .format = gpu::VertexElementFormat::Float4,
                             .offset = offset});
    offset += sizeof(float) * 4;
    vertexInputState.vertexAttributes.emplace_back(
        gpu::VertexAttribute{.location = location++,
                             .bufferSlot = bufferSlot,
                             .format = gpu::VertexElementFormat::Float4,
                             .offset = offset});
    offset += sizeof(float) * 4;
    vertexInputState.vertexAttributes.emplace_back(
        gpu::VertexAttribute{.location = location++,
                             .bufferSlot = bufferSlot,
                             .format = gpu::VertexElementFormat::Float4,
                             .offset = offset});
  }
  if (isAnimation) {
    bufferSlot++;
    vertexInputState.vertexBufferDescriptions.emplace_back(
        gpu::VertexBufferDescription{
            .slot = bufferSlot,
            .pitch = sizeof(SkinnedVertex),
            .inputRate = gpu::VertexInputRate::Vertex,
            .instanceStepRate = 0,
        });
    vertexInputState.vertexAttributes.emplace_back(
        gpu::VertexAttribute{.location = location++,
                             .bufferSlot = bufferSlot,
                             .format = gpu::VertexElementFormat::Float4,
                             .offset = offsetof(SkinnedVertex, boneIDs)});
    vertexInputState.vertexAttributes.emplace_back(
        gpu::VertexAttribute{.location = location++,
                             .bufferSlot = bufferSlot,
                             .format = gpu::VertexElementFormat::Float4,
                             .offset = offsetof(SkinnedVertex, boneWeights)});
  }
  if (isTangent) {
    bufferSlot++;
    vertexInputState.vertexBufferDescriptions.emplace_back(
        gpu::VertexBufferDescription{
            .slot = bufferSlot,
            .pitch = sizeof(Vec4),
            .inputRate = gpu::VertexInputRate::Vertex,
            .instanceStepRate = 0,
        });
    vertexInputState.vertexAttributes.emplace_back(
        gpu::VertexAttribute{.location = location++,
                             .bufferSlot = bufferSlot,
                             .format = gpu::VertexElementFormat::Float4,
                             .offset = 0});
  }
  return vertexInputState;
}

static int lPipelineNew(lua_State *L) {
  udPushPtr<GraphicsPipeline>(L, makePtr<GraphicsPipeline>());
  return 1;
}
static int lPipelineSetVertexShader(lua_State *L) {
  auto &p = udPtr<GraphicsPipeline>(L, 1);
  auto &s = udPtr<Shader>(L, 2);
  p->setVertexShader(*s);
  return 0;
}
static int lPipelineSetFragmentShader(lua_State *L) {
  auto &p = udPtr<GraphicsPipeline>(L, 1);
  auto &s = udPtr<Shader>(L, 2);
  p->setFragmentShader(*s);
  return 0;
}
static int lPipelineSetEnableDepthTest(lua_State *L) {
  auto &p = udPtr<GraphicsPipeline>(L, 1);
  bool enable = lua_toboolean(L, 2) != 0;
  p->setEnableDepthTest(enable);
  return 0;
}
static int lPipelineSetEnableInstanced(lua_State *L) {
  auto &p = udPtr<GraphicsPipeline>(L, 1);
  bool enable = lua_toboolean(L, 2) != 0;
  p->setEnableInstanced(enable);
  return 0;
}
static int lPipelineSetEnableAnimation(lua_State *L) {
  auto &p = udPtr<GraphicsPipeline>(L, 1);
  bool enable = lua_toboolean(L, 2) != 0;
  p->setEnableAnimation(enable);
  return 0;
}
static int lPipelineSetEnableTangent(lua_State *L) {
  auto &p = udPtr<GraphicsPipeline>(L, 1);
  bool enable = lua_toboolean(L, 2) != 0;
  p->setEnableTangent(enable);
  return 0;
}
static int lPipelineBuild(lua_State *L) {
  udPtr<GraphicsPipeline>(L, 1)->build();
  return 0;
}
void registerPipeline(lua_State *L) {
  luaL_newmetatable(L, GraphicsPipeline::metaTableName());
  luaPushcfunction2(L, udPtrGc<GraphicsPipeline>);
  lua_setfield(L, -2, "__gc");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  luaPushcfunction2(L, lPipelineSetVertexShader);
  lua_setfield(L, -2, "setVertexShader");
  luaPushcfunction2(L, lPipelineSetFragmentShader);
  lua_setfield(L, -2, "setFragmentShader");
  luaPushcfunction2(L, lPipelineSetEnableDepthTest);
  lua_setfield(L, -2, "setEnableDepthTest");
  luaPushcfunction2(L, lPipelineSetEnableInstanced);
  lua_setfield(L, -2, "setEnableInstanced");
  luaPushcfunction2(L, lPipelineSetEnableAnimation);
  lua_setfield(L, -2, "setEnableAnimation");
  luaPushcfunction2(L, lPipelineSetEnableTangent);
  lua_setfield(L, -2, "setEnableTangent");
  luaPushcfunction2(L, lPipelineBuild);
  lua_setfield(L, -2, "build");
  lua_pop(L, 1);

  pushSnNamed(L, "GraphicsPipeline");
  luaPushcfunction2(L, lPipelineNew);
  lua_setfield(L, -2, "new");
  lua_pop(L, 1);
}
} // namespace sinen
