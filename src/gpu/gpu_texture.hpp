#ifndef SINEN_GPU_TEXTURE_HPP
#define SINEN_GPU_TEXTURE_HPP
#include <core/allocator/allocator.hpp>
#include <core/def/types.hpp>
namespace sinen::gpu {
enum class TextureFormat {
  Invalid,
  R8_UNORM,
  R8G8_UNORM,
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
enum class SampleCount {
  x1,
  x2,
  x4,
  x8,
};
class Texture {
public:
  struct CreateInfo {
    Allocator *allocator;
    TextureType type;
    TextureFormat format;
    TextureUsage usage;
    UInt32 width;
    UInt32 height;
    UInt32 layerCountOrDepth;
    UInt32 numLevels;
    SampleCount sampleCount;
  };
  virtual ~Texture() = default;

  const CreateInfo &getCreateInfo() const { return createInfo; }

protected:
  Texture(const CreateInfo &createInfo) : createInfo(createInfo) {}

private:
  CreateInfo createInfo;
};
} // namespace sinen::gpu
#endif
