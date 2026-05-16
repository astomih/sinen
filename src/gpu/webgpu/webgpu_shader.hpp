#ifndef SINEN_WEBGPU_SHADER_HPP
#define SINEN_WEBGPU_SHADER_HPP

#include "webgpu_api.hpp"
#include <core/data/ptr.hpp>
#include <gpu/gpu_shader.hpp>
#include <utility>
#include <vector>

namespace sinen::gpu::webgpu {
class Device;

class Shader : public gpu::Shader {
public:
  Shader(const CreateInfo &createInfo, const Ptr<Device> &device,
         WGPUShaderModule shaderModule)
      : gpu::Shader(createInfo), device(device), shaderModule(shaderModule),
        entrypoint(createInfo.entrypoint ? createInfo.entrypoint : "main") {
    initializeUniformBindings(createInfo);
  }
  ~Shader() override;

  WGPUShaderModule getNative() const { return shaderModule; }
  const char *getEntryPoint() const { return entrypoint; }
  bool hasKnownUniformBindings() const { return knownUniformBindings; }
  bool hasUniformBinding(UInt32 group, UInt32 binding) const;
  bool hasResourceBinding(UInt32 group, UInt32 binding) const;

private:
  Ptr<Device> device;
  WGPUShaderModule shaderModule;
  const char *entrypoint;
  bool knownUniformBindings = false;
  std::vector<std::pair<UInt32, UInt32>> uniformBindings;
  std::vector<std::pair<UInt32, UInt32>> resourceBindings;

  void initializeUniformBindings(const CreateInfo &createInfo);
};
} // namespace sinen::gpu::webgpu

#endif // SINEN_WEBGPU_SHADER_HPP
