#ifndef SINEN_GPU_SHADER_HPP
#define SINEN_GPU_SHADER_HPP
#include <core/allocator/allocator.hpp>
#include <core/data/ptr.hpp>
#include <core/def/types.hpp>
#include <graphics/shader/shader_format.hpp>
#include <graphics/shader/shader_stage.hpp>
namespace sinen::gpu {
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
  const CreateInfo &getCreateInfo() const { return createInfo; }

protected:
  Shader(const CreateInfo &createInfo) : createInfo(createInfo) {}

private:
  CreateInfo createInfo;
};
} // namespace sinen::gpu
#endif
