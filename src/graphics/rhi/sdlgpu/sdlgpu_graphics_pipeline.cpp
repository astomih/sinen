#include "sdlgpu_graphics_pipeline.hpp"

#ifndef EMSCRIPTEN
#include "sdlgpu_convert.hpp"
#include "sdlgpu_device.hpp"
#include "sdlgpu_shader.hpp"
#include "sdlgpu_texture.hpp"

#include <SDL3/SDL_gpu.h>

namespace sinen::rhi::sdlgpu {
GraphicsPipeline::~GraphicsPipeline() {
  SDL_ReleaseGPUGraphicsPipeline(device->getNative(), pipeline);
}
} // namespace sinen::rhi::sdlgpu
#endif // EMSCRIPTEN
