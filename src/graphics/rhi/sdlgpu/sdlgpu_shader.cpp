#include "sdlgpu_shader.hpp"

#ifndef EMSCRIPTEN
#include "sdlgpu_device.hpp"

#include <SDL3/SDL_gpu.h>

namespace sinen::rhi::sdlgpu {
Shader::~Shader() { SDL_ReleaseGPUShader(device->getNative(), shader); }
} // namespace sinen::rhi::sdlgpu
#endif // EMSCRIPTEN
