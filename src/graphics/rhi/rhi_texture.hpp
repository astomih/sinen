#ifndef SINEN_RHI_TEXTURE_HPP
#define SINEN_RHI_TEXTURE_HPP
#include <core/allocator/allocator.hpp>
#include <core/def/types.hpp>
namespace sinen::rhi {
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
} // namespace sinen::rhi
#endif