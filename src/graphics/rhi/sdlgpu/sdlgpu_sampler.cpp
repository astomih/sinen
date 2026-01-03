#include "sdlgpu_sampler.hpp"

#ifndef EMSCRIPTEN
#include "sdlgpu_device.hpp"

#include <SDL3/SDL_gpu.h>

namespace sinen::rhi::sdlgpu {
Sampler::~Sampler() { SDL_ReleaseGPUSampler(device->getNative(), sampler); }
} // namespace sinen::rhi::sdlgpu
#endif // EMSCRIPTEN
