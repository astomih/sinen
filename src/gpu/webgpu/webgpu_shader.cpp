#include "webgpu_shader.hpp"

#ifndef EMSCRIPTEN

namespace sinen::gpu::webgpu {
Shader::~Shader() {
  if (shaderModule) {
    wgpuShaderModuleRelease(shaderModule);
    shaderModule = nullptr;
  }
}
} // namespace sinen::gpu::webgpu

#endif // EMSCRIPTEN
