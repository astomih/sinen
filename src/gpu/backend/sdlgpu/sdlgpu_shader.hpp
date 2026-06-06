#ifndef EMSCRIPTEN
#ifndef SINEN_SDLGPU_SHADER_HPP
#define SINEN_SDLGPU_SHADER_HPP

#include <core/data/ptr.hpp>
#include <core/data/array.hpp>
#include <gpu/gpu_shader.hpp>
#include <SDL3/SDL_gpu.h>
#include <utility>

namespace sinen::gpu::sdlgpu {
class Device;

class Shader : public gpu::Shader {
public:
  Shader(const CreateInfo &createInfo, const Ptr<Device> &device,
         SDL_GPUShader *shader, Array<Uint8> bytecode = {})
      : gpu::Shader(createInfo), device(device), shader(shader),
        bytecode(std::move(bytecode)) {}
  ~Shader() override;

  SDL_GPUShader *getNative() { return shader; }
  const Array<Uint8> &getBytecode() const { return bytecode; }

private:
  Ptr<Device> device;
  SDL_GPUShader *shader;
  Array<Uint8> bytecode;
};
} // namespace sinen::gpu::sdlgpu

#endif // SINEN_SDLGPU_SHADER_HPP
#endif // EMSCRIPTEN
