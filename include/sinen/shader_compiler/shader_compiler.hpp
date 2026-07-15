#ifndef SINEN_SHADER_COMPILER_HPP
#define SINEN_SHADER_COMPILER_HPP
#include <core/data/array.hpp>
#include <core/data/string.hpp>
#include <gpu/shader/shader_format.hpp>
#include <gpu/shader/shader_stage.hpp>

#include <cstdint>

namespace sinen {
class ShaderCompiler {
public:
  struct ResourceBinding {
    String name;
    uint32_t slot = 0;
  };

  struct ReflectionData {
    uint32_t numUniformBuffers = 0;
    uint32_t numCombinedSamplers = 0;
    uint32_t numStorageBuffers = 0;
    uint32_t numStorageTextures = 0;
    Array<ResourceBinding> uniformBuffers;
    Array<ResourceBinding> textures;
  };

  ShaderCompiler() = default;

  Array<char> compile(StringView sourcePath, ShaderStage stage,
                      ShaderFormat format, ReflectionData &reflectionData);
  Array<char> compileSource(StringView moduleName, StringView modulePath,
                            StringView source, ShaderStage stage,
                            ShaderFormat format,
                            ReflectionData &reflectionData);
};
} // namespace sinen

#endif // SINEN_SHADER_COMPILER_HPP
