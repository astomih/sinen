#ifndef EMSCRIPTEN
#ifndef SINEN_WEBGPU_TRANSFER_BUFFER_HPP
#define SINEN_WEBGPU_TRANSFER_BUFFER_HPP

#include <core/data/ptr.hpp>
#include <gpu/gpu_transfer_buffer.hpp>
#include <vector>
#include <webgpu/webgpu.h>

namespace sinen::gpu::webgpu {
class Device;

class TransferBuffer : public gpu::TransferBuffer {
public:
  TransferBuffer(const CreateInfo &createInfo, const Ptr<Device> &device,
                 WGPUBuffer transferBuffer)
      : gpu::TransferBuffer(createInfo), device(device),
        transferBuffer(transferBuffer), uploadData(createInfo.size) {}
  ~TransferBuffer() override;

  WGPUBuffer getNative() const { return transferBuffer; }
  const UInt8 *getUploadData() const {
    return uploadData.empty() ? nullptr : uploadData.data();
  }

  void *map(bool cycle) override;
  void unmap() override;

private:
  struct MapState {
    bool done = false;
    bool success = false;
  };

  static void onMapComplete(WGPUMapAsyncStatus status, WGPUStringView message,
                            void *userdata1, void *userdata2);

  Ptr<Device> device;
  WGPUBuffer transferBuffer;
  std::vector<UInt8> uploadData;
};
} // namespace sinen::gpu::webgpu

#endif // SINEN_WEBGPU_TRANSFER_BUFFER_HPP
#endif // EMSCRIPTEN
