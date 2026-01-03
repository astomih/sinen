#ifndef EMSCRIPTEN
#ifndef SINEN_SDLGPU_COMPUTE_PIPELINE_HPP
#define SINEN_SDLGPU_COMPUTE_PIPELINE_HPP

#include <core/data/ptr.hpp>
#include <graphics/rhi/rhi_compute_pipeline.hpp>
#include <SDL3/SDL_gpu.h>

namespace sinen::rhi::sdlgpu {
class Device;

class ComputePipeline : public rhi::ComputePipeline {
public:
  ComputePipeline() : rhi::ComputePipeline({}) {}
  ComputePipeline(const CreateInfo &createInfo, const Ptr<Device> &device,
                  SDL_GPUComputePipeline *pipeline)
      : rhi::ComputePipeline(createInfo), device(device), pipeline(pipeline) {}
  ~ComputePipeline() override {}

private:
  Ptr<Device> device;
  SDL_GPUComputePipeline *pipeline;
};
} // namespace sinen::rhi::sdlgpu

#endif // SINEN_SDLGPU_COMPUTE_PIPELINE_HPP
#endif // EMSCRIPTEN
