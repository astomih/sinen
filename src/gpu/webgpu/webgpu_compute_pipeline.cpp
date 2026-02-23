#include "webgpu_compute_pipeline.hpp"

namespace sinen::gpu::webgpu {
ComputePipeline::~ComputePipeline() {
  if (pipeline) {
    wgpuComputePipelineRelease(pipeline);
    pipeline = nullptr;
  }
}
} // namespace sinen::gpu::webgpu