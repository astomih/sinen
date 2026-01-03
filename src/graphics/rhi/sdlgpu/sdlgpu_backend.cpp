#include "sdlgpu_backend.hpp"
#include "sdlgpu_device.hpp"

#ifndef EMSCRIPTEN
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

namespace sinen::rhi::sdlgpu {
Ptr<rhi::Device> Backend::createDevice(const Device::CreateInfo &createInfo) {
  SDL_GPUDevice *device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV,
                                              createInfo.debugMode, nullptr);
  if (!device) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Failed to create SDL_GPUDevice:\n");
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, SDL_GetError());
    return nullptr;
  }
  return makePtr<Device>(createInfo.allocator, createInfo, device);
}
} // namespace sinen::rhi::sdlgpu
#endif // EMSCRIPTEN
