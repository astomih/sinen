#ifndef EMSCRIPTEN
#ifndef SINEN_WEBGPU_COMPUTE_PIPELINE_HPP
#define SINEN_WEBGPU_COMPUTE_PIPELINE_HPP

#include <core/data/ptr.hpp>
#include <gpu/gpu_compute_pipeline.hpp>
#include <webgpu/webgpu.h>

namespace sinen::gpu::webgpu {
class Device;

class ComputePipeline : public gpu::ComputePipeline {
public:
  ComputePipeline() : gpu::ComputePipeline({}) {}
  ComputePipeline(const CreateInfo &createInfo, const Ptr<Device> &device,
                  WGPUComputePipeline pipeline)
      : gpu::ComputePipeline(createInfo), device(device), pipeline(pipeline) {}
  ~ComputePipeline() override;

private:
  Ptr<Device> device;
  WGPUComputePipeline pipeline;
};
} // namespace sinen::gpu::webgpu

#endif // SINEN_WEBGPU_COMPUTE_PIPELINE_HPP
#endif // EMSCRIPTEN
