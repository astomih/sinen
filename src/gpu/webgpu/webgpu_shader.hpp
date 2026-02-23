#ifndef EMSCRIPTEN
#ifndef SINEN_WEBGPU_SHADER_HPP
#define SINEN_WEBGPU_SHADER_HPP

#include <core/data/ptr.hpp>
#include <gpu/gpu_shader.hpp>
#include <webgpu/webgpu.h>

namespace sinen::gpu::webgpu {
class Device;

class Shader : public gpu::Shader {
public:
  Shader(const CreateInfo &createInfo, const Ptr<Device> &device,
         WGPUShaderModule shaderModule)
      : gpu::Shader(createInfo), device(device), shaderModule(shaderModule),
        entrypoint(createInfo.entrypoint ? createInfo.entrypoint : "main") {}
  ~Shader() override;

  WGPUShaderModule getNative() const { return shaderModule; }
  const char *getEntryPoint() const { return entrypoint; }

private:
  Ptr<Device> device;
  WGPUShaderModule shaderModule;
  const char *entrypoint;
};
} // namespace sinen::gpu::webgpu

#endif // SINEN_WEBGPU_SHADER_HPP
#endif // EMSCRIPTEN
