#ifndef SINEN_WEBGPU_GRAPHICS_PIPELINE_HPP
#define SINEN_WEBGPU_GRAPHICS_PIPELINE_HPP

#include <core/data/ptr.hpp>
#include <gpu/gpu_graphics_pipeline.hpp>
#include <webgpu/webgpu.h>

namespace sinen::gpu::webgpu {
class Device;

class GraphicsPipeline : public gpu::GraphicsPipeline {
public:
  GraphicsPipeline(const CreateInfo &createInfo, const Ptr<Device> &device,
                   WGPURenderPipeline pipeline)
      : gpu::GraphicsPipeline(createInfo), device(device), pipeline(pipeline) {}
  ~GraphicsPipeline() override;

  WGPURenderPipeline getNative() const { return pipeline; }

private:
  Ptr<Device> device;
  WGPURenderPipeline pipeline;
};
} // namespace sinen::gpu::webgpu

#endif // SINEN_WEBGPU_GRAPHICS_PIPELINE_HPP