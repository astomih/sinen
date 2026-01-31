#include "sdlgpu_shader.hpp"

#ifndef EMSCRIPTEN
#include "sdlgpu_device.hpp"

#include <SDL3/SDL_gpu.h>

namespace sinen::gpu::sdlgpu {
Shader::~Shader() { SDL_ReleaseGPUShader(device->getNative(), shader); }
} // namespace sinen::gpu::sdlgpu
#endif // EMSCRIPTEN
