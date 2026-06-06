#include "webgpu_sampler.hpp"

namespace sinen::gpu::webgpu {
Sampler::~Sampler() {
  if (sampler) {
    wgpuSamplerRelease(sampler);
    sampler = nullptr;
  }
}
} // namespace sinen::gpu::webgpu