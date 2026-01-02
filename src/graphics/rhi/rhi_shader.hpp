#ifndef SINEN_RHI_SHADER_HPP
#define SINEN_RHI_SHADER_HPP
#include <core/allocator/allocator.hpp>
#include <core/data/ptr.hpp>
#include <core/def/types.hpp>
namespace sinen::rhi {
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
} // namespace sinen::rhi
#endif