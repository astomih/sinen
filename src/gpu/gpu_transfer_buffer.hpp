#ifndef SINEN_GPU_TRANSFER_BUFFER_HPP
#define SINEN_GPU_TRANSFER_BUFFER_HPP
#include <core/allocator/allocator.hpp>
#include <core/data/ptr.hpp>
#include <core/def/types.hpp>
namespace sinen::gpu {
enum class TransferBufferUsage { Upload, Download };
class TransferBuffer {
public:
  struct CreateInfo {
    Allocator *allocator;
    TransferBufferUsage usage;
    UInt32 size;
  };
  virtual ~TransferBuffer() = default;

  const CreateInfo &getCreateInfo() const { return createInfo; }

  virtual void *map(bool cycle) = 0;
  virtual void unmap() = 0;

protected:
  TransferBuffer(const CreateInfo &createInfo) : createInfo(createInfo) {}

private:
  CreateInfo createInfo;
};
} // namespace sinen::gpu
#endif
