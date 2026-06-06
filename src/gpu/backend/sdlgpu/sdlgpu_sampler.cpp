#include "sdlgpu_sampler.hpp"

#ifndef EMSCRIPTEN
#include "sdlgpu_device.hpp"

#include <SDL3/SDL_gpu.h>

namespace sinen::gpu::sdlgpu {
Sampler::~Sampler() { SDL_ReleaseGPUSampler(device->getNative(), sampler); }
} // namespace sinen::gpu::sdlgpu
#endif // EMSCRIPTEN
