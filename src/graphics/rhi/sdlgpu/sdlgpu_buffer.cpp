#include "sdlgpu_buffer.hpp"

#ifndef EMSCRIPTEN
#include "sdlgpu_device.hpp"

#include <SDL3/SDL_gpu.h>

namespace sinen::rhi::sdlgpu {
Buffer::~Buffer() { SDL_ReleaseGPUBuffer(device->getNative(), buffer); }
} // namespace sinen::rhi::sdlgpu
#endif // EMSCRIPTEN
