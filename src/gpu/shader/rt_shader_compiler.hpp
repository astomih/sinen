#ifndef SINEN_SHADER_COMPILER_HPP
#define SINEN_SHADER_COMPILER_HPP
#include "shader_format.hpp"
#include "shader_stage.hpp"
#include <core/data/array.hpp>
#include <core/data/string.hpp>

namespace sinen {
class ShaderCompiler {
public:
  struct ReflectionData {
    uint32_t numUniformBuffers;
    uint32_t numCombinedSamplers;
  };

  ShaderCompiler() = default;

  Array<char> compile(StringView sourcePath, ShaderStage stage,
                      ShaderFormat format, ReflectionData &reflectionData);
};
} // namespace sinen

#endif // SINEN_SHADER_COMPILER_HPP