#ifndef SINEN_BUFFER_HPP
#define SINEN_BUFFER_HPP
#include <core/data/ptr.hpp>
#include <cstddef>
namespace sinen {
enum class BufferType {
  Unknown,
  Binary,
  String,
};
class Buffer {
public:
  Buffer(const BufferType &type, Ptr<void> data, size_t size);

  static constexpr const char *metaTableName() { return "sn.Buffer"; }

  Buffer(const Buffer &) = default;
  Buffer(Buffer &&) = default;
  Buffer &operator=(const Buffer &) = default;
  Buffer &operator=(Buffer &&) = default;

  int size() const;
  BufferType type() const;
  void *data() const;

private:
  size_t _size;
  BufferType _type;
  Ptr<void> _data;
};
Buffer makeBuffer(size_t size, BufferType type = BufferType::Unknown,
                  Allocator *allocator = GlobalAllocator::get());
Buffer makeBuffer(void *ptr, size_t size, BufferType type = BufferType::Unknown,
                  Allocator *allocator = GlobalAllocator::get());

} // namespace sinen
#endif
