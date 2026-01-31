#ifndef SINEN_GPU_SHADER_HPP
#define SINEN_GPU_SHADER_HPP
#include <core/allocator/allocator.hpp>
#include <core/data/ptr.hpp>
#include <core/def/types.hpp>
namespace sinen::gpu {
enum class ShaderFormat { SPIRV };
enum class ShaderStage { Vertex, Fragment };
class Shader {
public:
  struct CreateInfo {
    Allocator *allocator;
    size_t size;
    const void *data;
    const char *entrypoint;
    ShaderFormat format;
    ShaderStage stage;
    UInt32 numSamplers;
    UInt32 numStorageBuffers;
    UInt32 numStorageTextures;
    UInt32 numUniformBuffers;
  };
  virtual ~Shader() = default;

protected:
  Shader(const CreateInfo &createInfo) : createInfo(createInfo) {}

private:
  CreateInfo createInfo;
};
} // namespace sinen::gpu
#endif
