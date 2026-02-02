#ifndef SINEN_SHADER_HPP
#define SINEN_SHADER_HPP
#include "shader_stage.hpp"
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

private:
  struct AsyncState {
    std::future<void> future;
    Array<char> spirv;
    uint32_t numUniformBuffers = 0;
    uint32_t numSamplers = 0;
    gpu::ShaderStage gpuStage = gpu::ShaderStage::Vertex;
  };
  Ptr<Ptr<gpu::Shader>> shader;
  Ptr<AsyncState> async;
};
} // namespace sinen
#endif // !SINEN_SHADER_HPP
