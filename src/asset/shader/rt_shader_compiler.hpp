#ifndef RT_SHADER_COMPILER_HPP
#define RT_SHADER_COMPILER_HPP
#include <string_view>
#include <vector>

namespace rsc {
class ShaderCompiler {
public:
  enum class Type { VERTEX, FRAGMENT, COMPUTE };

  enum class Language {
    SPIRV,
    DXIL,
    WGSL,
  };
  ShaderCompiler() = default;

  std::vector<char> compile(std::string_view source, Type type, Language lang);
};
} // namespace rsc

#endif // RT_SHADER_COMPILER_HPP