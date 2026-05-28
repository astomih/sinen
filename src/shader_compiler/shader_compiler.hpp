#ifndef SINEN_SHADER_COMPILER_HPP
#define SINEN_SHADER_COMPILER_HPP
#include <gpu/shader_format.hpp>
#include <gpu/shader_stage.hpp>
#include <core/data/array.hpp>
#include <core/data/string.hpp>

namespace sinen {
class ShaderCompiler {
public:
  struct ReflectionData {
    uint32_t numUniformBuffers = 0;
    uint32_t numCombinedSamplers = 0;
    uint32_t numStorageBuffers = 0;
    uint32_t numStorageTextures = 0;
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
