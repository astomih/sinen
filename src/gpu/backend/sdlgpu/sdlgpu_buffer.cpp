#include "sdlgpu_buffer.hpp"

#ifndef EMSCRIPTEN
#include "sdlgpu_device.hpp"

#include <SDL3/SDL_gpu.h>

namespace sinen::gpu::sdlgpu {
Buffer::~Buffer() { SDL_ReleaseGPUBuffer(device->getNative(), buffer); }
} // namespace sinen::gpu::sdlgpu
#endif // EMSCRIPTEN
