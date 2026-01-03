#ifndef EMSCRIPTEN
#ifndef SINEN_SDLGPU_SAMPLER_HPP
#define SINEN_SDLGPU_SAMPLER_HPP

#include <core/data/ptr.hpp>
#include <graphics/rhi/rhi_sampler.hpp>
#include <SDL3/SDL_gpu.h>

namespace sinen::rhi::sdlgpu {
class Device;

class Sampler : public rhi::Sampler {
public:
  Sampler(const CreateInfo &createInfo, const Ptr<Device> &device,
          SDL_GPUSampler *sampler)
      : rhi::Sampler(createInfo), device(device), sampler(sampler) {}
  ~Sampler() override;

  SDL_GPUSampler *getNative() const { return sampler; }

private:
  Ptr<Device> device;
  SDL_GPUSampler *sampler;
};
} // namespace sinen::rhi::sdlgpu

#endif // SINEN_SDLGPU_SAMPLER_HPP
#endif // EMSCRIPTEN
