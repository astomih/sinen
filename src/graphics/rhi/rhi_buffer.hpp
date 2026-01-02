#ifndef SINEN_RHI_BUFFER_HPP
#define SINEN_RHI_BUFFER_HPP
#include <core/allocator/allocator.hpp>
#include <core/data/ptr.hpp>
#include <core/def/types.hpp>
namespace sinen::rhi {
enum class BufferUsage { Vertex, Index, Indirect };
class Buffer {
public:
  struct CreateInfo {
    Allocator *allocator;
    BufferUsage usage;
    uint32 size;
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
  uint32 offset;
  uint32 size;
};
struct BufferBinding {
  Ptr<Buffer> buffer;
  uint32 offset;
};
} // namespace sinen::rhi
#endif