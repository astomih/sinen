#ifndef PARANOIXA_HPP
#define PARANOIXA_HPP
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <memory>
#include <memory_resource>
#include <print>
#include <string>
#include <unordered_map>
#include <vector>

namespace paranoixa {
// Macro to define the build type
#ifdef _DEBUG
#define PARANOIXA_BUILD_DEBUG
#elif NDEBUG
#define PARANOIXA_BUILD_RELEASE
#endif

// Macro to define the platform
#ifdef _WIN32
#define PARANOIXA_PLATFORM_WINDOWS
#elif __linux__
#define PARANOIXA_PLATFORM_LINUX
#elif __APPLE__
#define PARANOIXA_PLATFORM_MACOS
#elif __EMSCRIPTEN__
#define PARANOIXA_PLATFORM_EMSCRIPTEN
#endif

// Shared pointer and reference type aliases
template <class T> using Ptr = std::shared_ptr<T>;
template <class T> using Ref = std::weak_ptr<T>;

using Allocator = std::pmr::memory_resource;

// Allocation wrapper functions
template <class T, class... Args>
Ptr<T> MakePtr(Allocator *allocator, Args &&...args) {
  return std::allocate_shared<T>(std::pmr::polymorphic_allocator<T>(allocator),
                                 std::forward<Args>(args)...);
}
template <class T, class U> Ptr<T> DownCast(Ptr<U> ptr) {
#ifdef PARANOIXA_BUILD_DEBUG
  return std::dynamic_pointer_cast<T>(ptr);
#else
  return std::static_pointer_cast<T>(ptr);
#endif
}

// Type aliases
using uint32 = std::uint32_t;
using uint8 = std::uint8_t;
using int32 = std::int32_t;
using int8 = std::int8_t;
using float32 = std::float_t;
using float64 = std::double_t;

// Array class
template <typename T> using Array = std::pmr::vector<T>;

using String = std::pmr::string;

// Hash map class
template <typename K, typename V, typename Hash = std::hash<K>,
          typename Equal = std::equal_to<K>>
using HashMap = std::pmr::unordered_map<K, V, Hash, Equal>;

enum class GraphicsAPI {
  Vulkan,
#ifdef PARANOIXA_PLATFORM_WINDOWS
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
struct BufferRegion {
  Ptr<class Buffer> buffer;
  uint32 offset;
  uint32 size;
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
struct BufferBinding {
  Ptr<class Buffer> buffer;
  uint32 offset;
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

class TransferBuffer {
public:
  struct CreateInfo {
    Allocator *allocator;
    TransferBufferUsage usage;
    uint32 size;
  };
  virtual ~TransferBuffer() = default;

  const CreateInfo &GetCreateInfo() const { return createInfo; }

  virtual void *Map(bool cycle) = 0;
  virtual void Unmap() = 0;

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

  virtual void UploadTexture(const TextureTransferInfo &src,
                             const TextureRegion &dst, bool cycle) = 0;
  virtual void DownloadTexture(const TextureRegion &src,
                               const TextureTransferInfo &dst) = 0;
  virtual void UploadBuffer(const BufferTransferInfo &src,
                            const BufferRegion &dst, bool cycle) = 0;
  virtual void DownloadBuffer(const BufferRegion &src,
                              const BufferTransferInfo &dst) = 0;

  virtual void CopyTexture(const TextureLocation &src,
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

  virtual void BindGraphicsPipeline(Ptr<GraphicsPipeline> graphicsPipeline) = 0;
  virtual void BindVertexBuffers(uint32 slot,
                                 const Array<BufferBinding> &bindings) = 0;
  virtual void BindIndexBuffer(const BufferBinding &binding,
                               IndexElementSize indexElementSize) = 0;
  virtual void
  BindFragmentSamplers(uint32 slot,
                       const Array<TextureSamplerBinding> &bindings) = 0;
  virtual void SetViewport(const Viewport &viewport) = 0;
  virtual void SetScissor(int32 x, int32 y, int32 width, int32 height) = 0;
  virtual void DrawPrimitives(uint32 numVertices, uint32 numInstances,
                              uint32 firstVertex, uint32 firstInstance) = 0;
  virtual void DrawIndexedPrimitives(uint32 numIndices, uint32 numInstances,
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

  inline CreateInfo GetCreateInfo() const { return createInfo; }

  virtual Ptr<class CopyPass> BeginCopyPass() = 0;
  virtual void EndCopyPass(Ptr<class CopyPass> copyPass) = 0;

  virtual Ptr<class RenderPass>
  BeginRenderPass(const Array<ColorTargetInfo> &infos,
                  const DepthStencilTargetInfo &depthStencilInfo, float r = 0.f,
                  float g = 0.f, float b = 0.f, float a = 1.f) = 0;
  virtual void EndRenderPass(Ptr<RenderPass> renderPass) = 0;

  virtual void PushUniformData(uint32 slot, const void *data, size_t size) = 0;

  const CreateInfo &getCreateInfo() const { return createInfo; }

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
  const CreateInfo &GetCreateInfo() const { return createInfo; }

  /**
   * @brief Claim the SDL_Window for the device
   * @param window SDL_Window pointer
   */
  virtual void ClaimWindow(void *window) = 0;
  virtual Ptr<Buffer> CreateBuffer(const Buffer::CreateInfo &createInfo) = 0;
  virtual Ptr<Texture> CreateTexture(const Texture::CreateInfo &createInfo) = 0;
  virtual Ptr<Sampler> CreateSampler(const Sampler::CreateInfo &createInfo) = 0;
  virtual Ptr<TransferBuffer>
  CreateTransferBuffer(const TransferBuffer::CreateInfo &createInfo) = 0;
  virtual Ptr<Shader> CreateShader(const Shader::CreateInfo &createInfo) = 0;
  virtual Ptr<GraphicsPipeline>
  CreateGraphicsPipeline(const GraphicsPipeline::CreateInfo &createInfo) = 0;
  virtual Ptr<ComputePipeline>
  CreateComputePipeline(const ComputePipeline::CreateInfo &createInfo) = 0;
  virtual Ptr<CommandBuffer>
  AcquireCommandBuffer(const CommandBuffer::CreateInfo &createInfo) = 0;
  virtual void SubmitCommandBuffer(Ptr<CommandBuffer> commandBuffer) = 0;
  virtual Ptr<Texture>
  AcquireSwapchainTexture(Ptr<CommandBuffer> commandBuffer) = 0;
  virtual TextureFormat GetSwapchainFormat() const = 0;
  virtual void WaitForGPUIdle() = 0;

  virtual String GetDriver() const = 0;

protected:
  Device(const CreateInfo &createInfo) : createInfo(createInfo) {}
  Ptr<Device> GetPtr() { return shared_from_this(); }

private:
  CreateInfo createInfo;
};

class Backend {
public:
  Backend() = default;
  virtual ~Backend() = default;
  virtual Ptr<Device> CreateDevice(const Device::CreateInfo &createInfo) = 0;
};

class Paranoixa {
public:
  static Ptr<Backend> CreateBackend(Allocator *allocator,
                                    const GraphicsAPI &api);
  static Allocator *CreateAllocator(size_t size);
};
} // namespace paranoixa
namespace px = paranoixa;
#endif
