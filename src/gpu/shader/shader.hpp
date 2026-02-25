#ifndef SINEN_SHADER_HPP
#define SINEN_SHADER_HPP
#include "shader_stage.hpp"
#include <core/buffer/buffer.hpp>
#include <core/data/array.hpp>
#include <core/data/ptr.hpp>
#include <core/data/string.hpp>
#include <gpu/gpu.hpp>

#include <cstdint>
#include <future>

namespace sinen {
/**
 * @brief Shader
 *
 */
class Shader {
public:
  Shader();
  Shader(const Ptr<gpu::Shader> &raw);

  static constexpr const char *metaTableName() { return "sn.Shader"; }

  void load(StringView name, ShaderStage stage, int numUniformData);
  void compileAndLoad(StringView name, ShaderStage stage);

  bool isReady() const { return shader != nullptr; }
  Ptr<gpu::Shader> getRaw();
  ShaderFormat getFormat() const;
  ShaderStage getStage() const;
  Buffer getCode() const;

private:
  struct AsyncState {
    std::future<void> future;
    Array<char> spirv;
    ShaderFormat shaderFormat = ShaderFormat::SPIRV;
    uint32_t numUniformBuffers = 0;
    uint32_t numSamplers = 0;
    ShaderStage gpuStage = ShaderStage::Vertex;
  };
  Ptr<Ptr<gpu::Shader>> shader;
  Ptr<AsyncState> async;
  ShaderFormat format = ShaderFormat::SPIRV;
  ShaderStage stage = ShaderStage::Vertex;
  Array<char> code;
};
} // namespace sinen
#endif // !SINEN_SHADER_HPP
