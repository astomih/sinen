#ifndef SINEN_SHADER_HPP
#define SINEN_SHADER_HPP
#include "shader_stage.hpp"
#include <gpu/gpu.hpp>


namespace sinen {
/**
 * @brief Shader
 *
 */
class Shader {
public:
  Shader() = default;
  Shader(const Ptr<gpu::Shader> &raw) : shader(raw) {}

  static constexpr const char *metaTableName() { return "sn.Shader"; }

  void load(StringView name, ShaderStage stage, int numUniformData);
  void compileAndLoad(StringView name, ShaderStage stage);

  Ptr<gpu::Shader> getRaw() { return shader; }

private:
  Ptr<gpu::Shader> shader;
};
} // namespace sinen
#endif // !SINEN_SHADER_HPP
