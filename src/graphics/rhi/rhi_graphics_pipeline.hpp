#ifndef SINEN_RHI_GRAPHICS_PIPELINE_HPP
#define SINEN_RHI_GRAPHICS_PIPELINE_HPP
#include "rhi_sampler.hpp"
#include "rhi_shader.hpp"
#include "rhi_texture.hpp"
#include <core/data/array.hpp>
#include <core/data/ptr.hpp>
namespace sinen::rhi {
struct ColorComponent {
  enum Type : uint8_t {
    R = (1u << 0),
    G = (1u << 1),
    B = (1u << 2),
    A = (1u << 3),

    RGB = R | G | B,
    RGBA = R | G | B | A,
  } type;
  ColorComponent(unsigned int v) : type(static_cast<Type>(v)) {}
  operator Type() { return type; }
};
enum class PrimitiveType {
  TriangleList,
  TriangleStrip,
  LineList,
  LineStrip,
  PointList,
};
enum class FillMode {
  Fill,
  Line,
};
enum class CullMode {
  None,
  Front,
  Back,
};
enum class FrontFace {
  Clockwise,
  CounterClockwise,
};
enum VertexInputRate {
  Vertex,
  Instance,
};
struct VertexBufferDescription {
  uint32 slot;
  uint32 pitch;
  VertexInputRate inputRate;
  uint32 instanceStepRate;
};
enum VertexElementFormat { Float1, Float2, Float3, Float4, UByte4_NORM };
struct VertexAttribute {
  uint32 location;
  uint32 bufferSlot;
  VertexElementFormat format;
  uint32 offset;
};
struct VertexInputState {
  VertexInputState(Allocator *allocator)
      : vertexBufferDescriptions(allocator), vertexAttributes(allocator) {}
  Array<VertexBufferDescription> vertexBufferDescriptions;
  Array<VertexAttribute> vertexAttributes;
};
struct RasterizerState {
  FillMode fillMode;
  CullMode cullMode;
  FrontFace frontFace;
  float depthBiasConstantFactor;
  float depthBiasClamp;
  float depthBiasSlopeFactor;
  bool enableDepthBias;
  bool enableDepthClip;
};
struct MultiSampleState {
  SampleCount sampleCount;
  uint32 sampleMask;
  bool enableMask;
};
enum class StencilOp {
  Invalid,
  Keep,
  Zero,
  Replace,
  IncrementAndClamp,
  DecrementAndClamp,
  Invert,
  IncrementAndWrap,
  DecrementAndWrap
};
struct StencilOpState {
  StencilOp failOp;
  StencilOp passOp;
  StencilOp depthFailOp;
  CompareOp compareOp;
};
struct DepthStencilState {
  CompareOp compareOp;
  StencilOpState backStencilState;
  StencilOpState frontStencilState;
  uint8 compareMask;
  uint8 writeMask;
  bool enableDepthTest;
  bool enableDepthWrite;
  bool enableStencilTest;
};
enum class LoadOp { Load, Clear, DontCare };
enum class StoreOp { Store, DontCare };
struct ColorTargetInfo {
  Ptr<Texture> texture;
  // clearColor
  LoadOp loadOp;
  StoreOp storeOp;
};
enum class BlendFactor {
  Zero,
  One,
  SrcColor,
  OneMinusSrcColor,
  DstColor,
  OneMinusDstColor,
  SrcAlpha,
  OneMinusSrcAlpha,
  DstAlpha,
  OneMinusDstAlpha,
  ConstantColor,
  OneMinusConstantColor,
  SrcAlphaSaturate,
};
enum class BlendOp {
  Add,
  Subtract,
  ReverseSubtract,
  Min,
  Max,
};
struct ColorTargetBlendState {
  BlendFactor srcColorBlendFactor;
  BlendFactor dstColorBlendFactor;
  BlendOp colorBlendOp;
  BlendFactor srcAlphaBlendFactor;
  BlendFactor dstAlphaBlendFactor;
  BlendOp alphaBlendOp;
  uint8_t colorWriteMask;
  bool enableBlend;
  bool enableColorWriteMask;
};
struct ColorTargetDescription {
  TextureFormat format;
  ColorTargetBlendState blendState;
};
struct TargetInfo {
  TargetInfo(Allocator *allocator)
      : colorTargetDescriptions(allocator),
        depthStencilTargetFormat(TextureFormat::Invalid),
        hasDepthStencilTarget(false) {}
  Array<ColorTargetDescription> colorTargetDescriptions;
  TextureFormat depthStencilTargetFormat;
  bool hasDepthStencilTarget;
};
class GraphicsPipeline {
public:
  struct CreateInfo {
    CreateInfo(Allocator *allocator)
        : allocator(allocator), vertexInputState(allocator),
          targetInfo(allocator), vertexShader(nullptr), fragmentShader(nullptr),
          primitiveType(PrimitiveType::TriangleList),
          rasterizerState{FillMode::Fill, CullMode::None, FrontFace::Clockwise,
                          0.0f,           0.0f,           0.0f,
                          false,          false},
          multiSampleState(), depthStencilState() {}
    Allocator *allocator;
    Ptr<Shader> vertexShader;
    Ptr<Shader> fragmentShader;
    VertexInputState vertexInputState;
    PrimitiveType primitiveType;
    RasterizerState rasterizerState;
    MultiSampleState multiSampleState;
    DepthStencilState depthStencilState;
    TargetInfo targetInfo;
  };
  virtual ~GraphicsPipeline() = default;

  const CreateInfo &getCreateInfo() const { return createInfo; }

protected:
  GraphicsPipeline(const CreateInfo &createInfo) : createInfo(createInfo) {}

private:
  CreateInfo createInfo;
};
} // namespace sinen::rhi
#endif