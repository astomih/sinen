#ifndef SINEN_GPU_BUFFER_HPP
#define SINEN_GPU_BUFFER_HPP
#include <core/allocator/allocator.hpp>
#include <core/data/ptr.hpp>
#include <core/def/types.hpp>
namespace sinen::gpu {
enum class BufferUsage { Vertex, Index, Indirect };
class Buffer {
public:
  struct CreateInfo {
    Allocator *allocator;
    BufferUsage usage;
    UInt32 size;
  };
  virtual ~Buffer() = default;

  const CreateInfo &getCreateInfo() const { return createInfo; }

protected:
  Buffer(const CreateInfo &createInfo) : createInfo(createInfo) {}

private:
  CreateInfo createInfo;
};
struct BufferRegion {
  Ptr<Buffer> buffer;
  UInt32 offset;
  UInt32 size;
};
struct BufferBinding {
  Ptr<Buffer> buffer;
  UInt32 offset;
};
} // namespace sinen::gpu
#endif
