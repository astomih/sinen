#ifndef SINEN_SHADER_BUNDLE_HPP
#define SINEN_SHADER_BUNDLE_HPP

#include <gpu/shader_format.hpp>
#include <gpu/shader_stage.hpp>
#include <core/buffer/buffer.hpp>
#include <core/data/array.hpp>
#include <core/data/string.hpp>
#include <gpu/gpu_backend_api.hpp>

#include <cstdint>
#include <optional>

namespace sinen {

class ShaderBundle {
public:
  struct Entry {
    ShaderStage stage = ShaderStage::Vertex;
    ShaderFormat format = ShaderFormat::SPIRV;
    Array<char> code;
    uint32_t numSamplers = 0;
    uint32_t numStorageBuffers = 0;
    uint32_t numStorageTextures = 0;
    uint32_t numUniformBuffers = 0;
  };

  struct PackEntry {
    ShaderStage stage = ShaderStage::Vertex;
    ShaderFormat format = ShaderFormat::SPIRV;
    const void *data = nullptr;
    uint32_t size = 0;
    uint32_t numSamplers = 0;
    uint32_t numStorageBuffers = 0;
    uint32_t numStorageTextures = 0;
    uint32_t numUniformBuffers = 0;
  };

  static bool isBundle(StringView data);
  static Array<char> pack(const Array<PackEntry> &entries);
  static std::optional<Entry> select(StringView data, ShaderStage stage,
                                     ShaderFormat preferredFormat);
  static ShaderFormat preferredFormatFor(GPUBackendAPI backendAPI);
  static const char *entryPointFor(ShaderStage stage, ShaderFormat format);
};

} // namespace sinen

#endif
