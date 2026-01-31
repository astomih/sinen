#ifndef EMSCRIPTEN
#ifndef SINEN_SDLGPU_SHADER_HPP
#define SINEN_SDLGPU_SHADER_HPP

#include <core/data/ptr.hpp>
#include <gpu/gpu_shader.hpp>
#include <SDL3/SDL_gpu.h>

namespace sinen::gpu::sdlgpu {
class Device;

class Shader : public gpu::Shader {
public:
  Shader(const CreateInfo &createInfo, const Ptr<Device> &device,
         SDL_GPUShader *shader)
      : gpu::Shader(createInfo), device(device), shader(shader) {}
  ~Shader() override;

  SDL_GPUShader *getNative() { return shader; }

private:
  Ptr<Device> device;
  SDL_GPUShader *shader;
};
} // namespace sinen::gpu::sdlgpu

#endif // SINEN_SDLGPU_SHADER_HPP
#endif // EMSCRIPTEN
