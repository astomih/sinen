#include "webgpu_compute_pipeline.hpp"

#ifndef EMSCRIPTEN

namespace sinen::gpu::webgpu {
ComputePipeline::~ComputePipeline() {
  if (pipeline) {
    wgpuComputePipelineRelease(pipeline);
    pipeline = nullptr;
  }
}
} // namespace sinen::gpu::webgpu

#endif // EMSCRIPTEN
