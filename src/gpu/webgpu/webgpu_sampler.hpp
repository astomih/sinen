#ifndef SINEN_WEBGPU_SAMPLER_HPP
#define SINEN_WEBGPU_SAMPLER_HPP

#include <core/data/ptr.hpp>
#include <gpu/gpu_sampler.hpp>
#include <webgpu/webgpu.h>

namespace sinen::gpu::webgpu {
class Device;

class Sampler : public gpu::Sampler {
public:
  Sampler(const CreateInfo &createInfo, const Ptr<Device> &device,
          WGPUSampler sampler)
      : gpu::Sampler(createInfo), device(device), sampler(sampler) {}
  ~Sampler() override;

  WGPUSampler getNative() const { return sampler; }

private:
  Ptr<Device> device;
  WGPUSampler sampler;
};
} // namespace sinen::gpu::webgpu

#endif // SINEN_WEBGPU_SAMPLER_HPP
