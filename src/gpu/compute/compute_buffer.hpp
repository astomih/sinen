#ifndef SINEN_COMPUTE_BUFFER_HPP
#define SINEN_COMPUTE_BUFFER_HPP

#include <core/buffer/buffer.hpp>
#include <gpu/gpu.hpp>

namespace sinen {
class ComputeBuffer {
public:
  static constexpr const char *metaTableName() { return "sn.ComputeBuffer"; }

  ComputeBuffer() = default;
  ~ComputeBuffer();

  explicit ComputeBuffer(UInt32 size);
  explicit ComputeBuffer(const Buffer &data);

  void create(UInt32 size);
  void upload(const Buffer &data);
  Buffer download() const;
  UInt32 size() const { return bufferSize; }

  Ptr<gpu::Buffer> getRaw() const { return buffer; }

private:
  Ptr<gpu::Buffer> buffer;
  UInt32 bufferSize = 0;
};
} // namespace sinen

#endif
