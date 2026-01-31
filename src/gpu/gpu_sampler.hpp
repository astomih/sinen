#ifndef SINEN_GPU_SAMPLER_HPP
#define SINEN_GPU_SAMPLER_HPP
#include <core/allocator/allocator.hpp>
#include <core/def/types.hpp>
namespace sinen::gpu {
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
} // namespace sinen::gpu
#endif
