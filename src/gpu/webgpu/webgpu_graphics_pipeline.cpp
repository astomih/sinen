#include "webgpu_graphics_pipeline.hpp"

namespace sinen::gpu::webgpu {
GraphicsPipeline::~GraphicsPipeline() {
  if (pipeline) {
    wgpuRenderPipelineRelease(pipeline);
    pipeline = nullptr;
  }
}
} // namespace sinen::gpu::webgpu
