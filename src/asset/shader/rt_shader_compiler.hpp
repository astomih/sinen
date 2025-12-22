#ifndef SINEN_SHADER_COMPILER_HPP
#define SINEN_SHADER_COMPILER_HPP
#include <core/data/array.hpp>
#include <core/data/string.hpp>

namespace sinen {
class ShaderCompiler {
public:
  enum class Type { VERTEX, FRAGMENT, COMPUTE };

  enum class Language {
    SPIRV,
    DXIL,
    WGSL,
  };

  struct ReflectionData {
    uint32_t numUniformBuffers;
    uint32_t numCombinedSamplers;
  };

  ShaderCompiler() = default;

  Array<char> compile(StringView sourcePath, Type type, Language lang,
                      ReflectionData &reflectionData);
};
} // namespace sinen

#endif // SINEN_SHADER_COMPILER_HPP