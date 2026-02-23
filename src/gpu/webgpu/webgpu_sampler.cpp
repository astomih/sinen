#include "webgpu_sampler.hpp"

#ifndef EMSCRIPTEN

namespace sinen::gpu::webgpu {
Sampler::~Sampler() {
  if (sampler) {
    wgpuSamplerRelease(sampler);
    sampler = nullptr;
  }
}
} // namespace sinen::gpu::webgpu

#endif // EMSCRIPTEN
