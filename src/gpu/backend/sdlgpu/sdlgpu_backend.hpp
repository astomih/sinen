#ifndef EMSCRIPTEN
#ifndef SINEN_SDLGPU_BACKEND_HPP
#define SINEN_SDLGPU_BACKEND_HPP

#include <gpu/gpu.hpp>

#include <SDL3/SDL_gpu.h>

namespace sinen::gpu::sdlgpu {
class Backend : public gpu::Backend {
public:
  Backend(SDL_GPUShaderFormat shaderFormats = SDL_GPU_SHADERFORMAT_SPIRV,
          const char *driverName = nullptr,
          GPUBackendAPI backendAPI = GPUBackendAPI::SDLGPU)
      : shaderFormats(shaderFormats), driverName(driverName),
        backendAPI(backendAPI) {}

  Ptr<gpu::Device>
  createDevice(const gpu::Device::CreateInfo &createInfo) override;

private:
  SDL_GPUShaderFormat shaderFormats;
  const char *driverName;
  GPUBackendAPI backendAPI;
};
} // namespace sinen::gpu::sdlgpu

#endif // SINEN_SDLGPU_BACKEND_HPP
#endif // EMSCRIPTEN
