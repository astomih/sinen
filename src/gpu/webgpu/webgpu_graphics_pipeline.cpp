#include "webgpu_graphics_pipeline.hpp"

#ifndef EMSCRIPTEN

namespace sinen::gpu::webgpu {
GraphicsPipeline::~GraphicsPipeline() {
  if (pipeline) {
    wgpuRenderPipelineRelease(pipeline);
    pipeline = nullptr;
  }
}
} // namespace sinen::gpu::webgpu

#endif // EMSCRIPTEN
