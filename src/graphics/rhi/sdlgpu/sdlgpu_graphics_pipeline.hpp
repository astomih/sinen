#ifndef EMSCRIPTEN
#ifndef SINEN_SDLGPU_GRAPHICS_PIPELINE_HPP
#define SINEN_SDLGPU_GRAPHICS_PIPELINE_HPP

#include <core/data/ptr.hpp>
#include <graphics/rhi/rhi_graphics_pipeline.hpp>
#include <SDL3/SDL_gpu.h>

namespace sinen::rhi::sdlgpu {
class Device;

class GraphicsPipeline : public rhi::GraphicsPipeline {
public:
  GraphicsPipeline(const CreateInfo &createInfo, const Ptr<Device> &device,
                   SDL_GPUGraphicsPipeline *pipeline)
      : rhi::GraphicsPipeline(createInfo), device(device), pipeline(pipeline) {}
  ~GraphicsPipeline() override;

  SDL_GPUGraphicsPipeline *getNative() { return pipeline; }

private:
  Ptr<Device> device;
  SDL_GPUGraphicsPipeline *pipeline;
};
} // namespace sinen::rhi::sdlgpu

#endif // SINEN_SDLGPU_GRAPHICS_PIPELINE_HPP
#endif // EMSCRIPTEN
