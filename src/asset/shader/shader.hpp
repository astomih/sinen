#ifndef SINEN_SHADER_HPP
#define SINEN_SHADER_HPP
#include "shader_stage.hpp"
#include <graphics/rhi/rhi.hpp>


namespace sinen {
/**
 * @brief Shader
 *
 */
class Shader {
public:
  Shader() = default;
  Shader(const Ptr<rhi::Shader> &raw) : shader(raw) {}

  static constexpr const char *metaTableName() { return "sn.Shader"; }

  void load(StringView name, ShaderStage stage, int numUniformData);
  void compileAndLoad(StringView name, ShaderStage stage);

  Ptr<rhi::Shader> getRaw() { return shader; }

private:
  Ptr<rhi::Shader> shader;
};
} // namespace sinen
#endif // !SINEN_SHADER_HPP
