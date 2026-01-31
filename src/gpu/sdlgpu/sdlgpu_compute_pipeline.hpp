#ifndef EMSCRIPTEN
#ifndef SINEN_SDLGPU_COMPUTE_PIPELINE_HPP
#define SINEN_SDLGPU_COMPUTE_PIPELINE_HPP

#include <core/data/ptr.hpp>
#include <gpu/gpu_compute_pipeline.hpp>
#include <SDL3/SDL_gpu.h>

namespace sinen::gpu::sdlgpu {
class Device;

class ComputePipeline : public gpu::ComputePipeline {
public:
  ComputePipeline() : gpu::ComputePipeline({}) {}
  ComputePipeline(const CreateInfo &createInfo, const Ptr<Device> &device,
                  SDL_GPUComputePipeline *pipeline)
      : gpu::ComputePipeline(createInfo), device(device), pipeline(pipeline) {}
  ~ComputePipeline() override {}

private:
  Ptr<Device> device;
  SDL_GPUComputePipeline *pipeline;
};
} // namespace sinen::gpu::sdlgpu

#endif // SINEN_SDLGPU_COMPUTE_PIPELINE_HPP
#endif // EMSCRIPTEN
