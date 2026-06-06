#include "sdlgpu_compute_pipeline.hpp"

#ifndef EMSCRIPTEN
#include "sdlgpu_device.hpp"

namespace sinen::gpu::sdlgpu {
ComputePipeline::~ComputePipeline() {
  if (pipeline) {
    SDL_ReleaseGPUComputePipeline(device->getNative(), pipeline);
    pipeline = nullptr;
  }
}
} // namespace sinen::gpu::sdlgpu
#endif // EMSCRIPTEN
