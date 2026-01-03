#ifndef EMSCRIPTEN
#ifndef SINEN_SDLGPU_SHADER_HPP
#define SINEN_SDLGPU_SHADER_HPP

#include <core/data/ptr.hpp>
#include <graphics/rhi/rhi_shader.hpp>
#include <SDL3/SDL_gpu.h>

namespace sinen::rhi::sdlgpu {
class Device;

class Shader : public rhi::Shader {
public:
  Shader(const CreateInfo &createInfo, const Ptr<Device> &device,
         SDL_GPUShader *shader)
      : rhi::Shader(createInfo), device(device), shader(shader) {}
  ~Shader() override;

  SDL_GPUShader *getNative() { return shader; }

private:
  Ptr<Device> device;
  SDL_GPUShader *shader;
};
} // namespace sinen::rhi::sdlgpu

#endif // SINEN_SDLGPU_SHADER_HPP
#endif // EMSCRIPTEN
