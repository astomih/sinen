#ifndef SINEN_RHI_HPP
#define SINEN_RHI_HPP
#include <cassert>
#include <core/data/array.hpp>
#include <core/data/hashmap.hpp>
#include <core/data/ptr.hpp>
#include <core/data/string.hpp>
#include <core/def/macro.hpp>
#include <core/def/types.hpp>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <memory>
#include <memory_resource>
#include <print>

namespace sinen::rhi {

enum class GraphicsAPI {
  Vulkan,
#ifdef SINEN_PLATFORM_WINDOWS
  D3D12U,
#endif
  WebGPU,
  SDLGPU,
};

enum class ShaderFormat { SPIRV };
enum class ShaderStage { Vertex, Fragment };
enum class TransferBufferUsage { Upload, Download };
enum class TextureFormat {
  Invalid,
  R8G8B8A8_UNORM,
  B8G8R8A8_UNORM,
  R32G32B32A32_FLOAT,
  D32_FLOAT_S8_UINT
};
enum class TextureUsage { Sampler, ColorTarget, DepthStencilTarget };
enum class TextureType {
  Texture2D,
  Texture2DArray,
  Texture3D,
  Cube,
  CubeArray
};
enum class BufferUsage { Vertex, Index, Indirect };
enum class SampleCount {
  x1,
  x2,
  x4,
  x8,
};
enum class Filter { Nearest, Linear };
enum class MipmapMode { Nearest, Linear };
enum class AddressMode { Repeat, MirroredRepeat, ClampToEdge };
enum class CompareOp {
  Invalid,
  Never,
  Less,
  Equal,
  LessOrEqual,
  Greater,
  NotEqual,
  GreaterOrEqual,
  Always
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
enum class LoadOp { Load, Clear, DontCare };
enum class StoreOp { Store, DontCare };
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
enum class IndexElementSize { Uint16, Uint32 };
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
class Device;
class Texture {
public:
  struct CreateInfo {
    Allocator *allocator;
    TextureType type;
    TextureFormat format;
    TextureUsage usage;
    uint32 width;
    uint32 height;
    uint32 layerCountOrDepth;
    uint32 numLevels;
    SampleCount sampleCount;
  };
  virtual ~Texture() = default;

  const CreateInfo &getCreateInfo() const { return createInfo; }

protected:
  Texture(const CreateInfo &createInfo) : createInfo(createInfo) {}

private:
  CreateInfo createInfo;
};
struct TextureTransferInfo {
  Ptr<class TransferBuffer> transferBuffer;
  uint32 offset;
};
struct TextureRegion {
  Ptr<class Texture> texture;
  uint32 mipLevel;
  uint32 layer;
  uint32 x, y, z;
  uint32 width;
  uint32 height;
  uint32 depth;
};
struct TextureLocation {
  Ptr<Texture> texture;
  uint32 mipLevel;
  uint32 layer;
  uint32 x;
  uint32 y;
  uint32 z;
};
struct BufferTransferInfo {
  Ptr<class TransferBuffer> transferBuffer;
  uint32 offset;
};
struct ColorTargetInfo {
  Ptr<class Texture> texture;
  // clearColor
  LoadOp loadOp;
  StoreOp storeOp;
};
struct DepthStencilTargetInfo {
  Ptr<class Texture> texture;
  float clearDepth;
  LoadOp loadOp;
  StoreOp storeOp;
  LoadOp stencilLoadOp;
  StoreOp stencilStoreOp;
  bool cycle;
  uint8 clearStencil;
};
struct TextureSamplerBinding {
  Ptr<class Sampler> sampler;
  Ptr<class Texture> texture;
};
struct MultiSampleState {
  SampleCount sampleCount;
  uint32 sampleMask;
  bool enableMask;
};

class Sampler {
public:
  struct CreateInfo {
    Allocator *allocator;
    Filter minFilter;
    Filter magFilter;
    MipmapMode mipmapMode;
    AddressMode addressModeU;
    AddressMode addressModeV;
    AddressMode addressModeW;
    float mipLodBias;
    float maxAnisotropy;
    CompareOp compareOp;
    float minLod;
    float maxLod;
    bool enableAnisotropy;
    bool enableCompare;
  };
  Sampler(const CreateInfo &createInfo) : createInfo(createInfo) {}
  virtual ~Sampler() = default;

private:
  CreateInfo createInfo;
};

class Buffer {
public:
  struct CreateInfo {
    Allocator *allocator;
    BufferUsage usage;
    uint32 size;
  };
  virtual ~Buffer() = default;

  const CreateInfo &getCreateInfo() const { return createInfo; }

protected:
  Buffer(const CreateInfo &createInfo) : createInfo(createInfo) {}

private:
  CreateInfo createInfo;
};
struct BufferRegion {
  Ptr<Buffer> buffer;
  uint32 offset;
  uint32 size;
};
struct BufferBinding {
  Ptr<Buffer> buffer;
  uint32 offset;
};

class TransferBuffer {
public:
  struct CreateInfo {
    Allocator *allocator;
    TransferBufferUsage usage;
    uint32 size;
  };
  virtual ~TransferBuffer() = default;

  const CreateInfo &getCreateInfo() const { return createInfo; }

  virtual void *map(bool cycle) = 0;
  virtual void unmap() = 0;

protected:
  TransferBuffer(const CreateInfo &createInfo) : createInfo(createInfo) {}

private:
  CreateInfo createInfo;
};

class Shader {
public:
  struct CreateInfo {
    Allocator *allocator;
    size_t size;
    const void *data;
    const char *entrypoint;
    ShaderFormat format;
    ShaderStage stage;
    uint32 numSamplers;
    uint32 numStorageBuffers;
    uint32 numStorageTextures;
    uint32 numUniformBuffers;
  };
  virtual ~Shader() = default;

protected:
  Shader(const CreateInfo &createInfo) : createInfo(createInfo) {}

private:
  CreateInfo createInfo;
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

class ComputePipeline {
public:
  struct CreateInfo {
    Allocator *allocator;
    Ptr<Shader> computeShader;
  };
  virtual ~ComputePipeline() = default;

protected:
  ComputePipeline(const CreateInfo &createInfo) : createInfo(createInfo) {}

private:
  CreateInfo createInfo;
};

class CopyPass {
public:
  virtual ~CopyPass() = default;

  virtual void uploadTexture(const TextureTransferInfo &src,
                             const TextureRegion &dst, bool cycle) = 0;
  virtual void downloadTexture(const TextureRegion &src,
                               const TextureTransferInfo &dst) = 0;
  virtual void uploadBuffer(const BufferTransferInfo &src,
                            const BufferRegion &dst, bool cycle) = 0;
  virtual void downloadBuffer(const BufferRegion &src,
                              const BufferTransferInfo &dst) = 0;

  virtual void copyTexture(const TextureLocation &src,
                           const TextureLocation &dst, uint32 width,
                           uint32 height, uint32 depth, bool cycle) = 0;
};
struct Viewport {
  float x;
  float y;
  float width;
  float height;
  float minDepth;
  float maxDepth;
};
class RenderPass {
public:
  virtual ~RenderPass() = default;

  virtual void bindGraphicsPipeline(Ptr<GraphicsPipeline> graphicsPipeline) = 0;
  virtual void bindVertexBuffers(uint32 slot,
                                 const Array<BufferBinding> &bindings) = 0;
  virtual void bindIndexBuffer(const BufferBinding &binding,
                               IndexElementSize indexElementSize) = 0;
  virtual void
  bindFragmentSamplers(uint32 slot,
                       const Array<TextureSamplerBinding> &bindings) = 0;
  virtual void setViewport(const Viewport &viewport) = 0;
  virtual void setScissor(int32 x, int32 y, int32 width, int32 height) = 0;
  virtual void drawPrimitives(uint32 numVertices, uint32 numInstances,
                              uint32 firstVertex, uint32 firstInstance) = 0;
  virtual void drawIndexedPrimitives(uint32 numIndices, uint32 numInstances,
                                     uint32 firstIndex, uint32 vertexOffset,
                                     uint32 firstInstance) = 0;

protected:
  RenderPass() = default;
};

class CommandBuffer {
public:
  struct CreateInfo {
    Allocator *allocator;
  };
  virtual ~CommandBuffer() = default;

  inline const CreateInfo &getCreateInfo() const { return createInfo; }

  virtual Ptr<class CopyPass> beginCopyPass() = 0;
  virtual void endCopyPass(Ptr<class CopyPass> copyPass) = 0;

  virtual Ptr<class RenderPass>
  beginRenderPass(const Array<ColorTargetInfo> &infos,
                  const DepthStencilTargetInfo &depthStencilInfo, float r = 0.f,
                  float g = 0.f, float b = 0.f, float a = 1.f) = 0;
  virtual void endRenderPass(Ptr<RenderPass> renderPass) = 0;

  virtual void pushUniformData(uint32 slot, const void *data, size_t size) = 0;

protected:
  CommandBuffer(const CreateInfo &createInfo) : createInfo(createInfo) {}

private:
  CreateInfo createInfo;
};

class Device : public std::enable_shared_from_this<Device> {
public:
  struct CreateInfo {
    Allocator *allocator;
    bool debugMode;
  };
  virtual ~Device() = default;
  const CreateInfo &getCreateInfo() const { return createInfo; }

  /**
   * @brief Claim the SDL_Window for the device
   * @param window SDL_Window pointer
   */
  virtual void claimWindow(void *window) = 0;
  virtual Ptr<Buffer> createBuffer(const Buffer::CreateInfo &createInfo) = 0;
  virtual Ptr<Texture> createTexture(const Texture::CreateInfo &createInfo) = 0;
  virtual Ptr<Sampler> createSampler(const Sampler::CreateInfo &createInfo) = 0;
  virtual Ptr<TransferBuffer>
  createTransferBuffer(const TransferBuffer::CreateInfo &createInfo) = 0;
  virtual Ptr<Shader> createShader(const Shader::CreateInfo &createInfo) = 0;
  virtual Ptr<GraphicsPipeline>
  createGraphicsPipeline(const GraphicsPipeline::CreateInfo &createInfo) = 0;
  virtual Ptr<ComputePipeline>
  createComputePipeline(const ComputePipeline::CreateInfo &createInfo) = 0;
  virtual Ptr<CommandBuffer>
  acquireCommandBuffer(const CommandBuffer::CreateInfo &createInfo) = 0;
  virtual void submitCommandBuffer(Ptr<CommandBuffer> commandBuffer) = 0;
  virtual Ptr<Texture>
  acquireSwapchainTexture(Ptr<CommandBuffer> commandBuffer) = 0;
  virtual TextureFormat getSwapchainFormat() const = 0;
  virtual void waitForGpuIdle() = 0;

  virtual String getDriver() const = 0;

protected:
  Device(const CreateInfo &createInfo) : createInfo(createInfo) {}
  Ptr<Device> getPtr() { return shared_from_this(); }

private:
  CreateInfo createInfo;
};

class Backend {
public:
  Backend() = default;
  virtual ~Backend() = default;
  virtual Ptr<Device> createDevice(const Device::CreateInfo &createInfo) = 0;
};

class RHI {
public:
  static Ptr<Backend> createBackend(Allocator *allocator,
                                    const GraphicsAPI &api);
};
} // namespace sinen::rhi
#endif
