#include "buffer.hpp"
#include <cstddef>

namespace sinen {
Buffer::Buffer(const BufferType &type, Ptr<void> data, size_t size)
    : _size(size), _type(type), _data(data) {}
int Buffer::size() const { return _size; }
BufferType Buffer::type() const { return _type; }
void *Buffer::data() const { return _data.get(); }
Buffer makeBuffer(size_t size, BufferType type, Allocator *allocator) {
  auto *ptr = allocator->allocate(size);
  auto deleter = Deleter<void>(allocator, size);
  return Buffer(type, Ptr<void>(ptr, std::move(deleter)), size);
}
Buffer makeBuffer(void *ptr, size_t size, BufferType type,
                  Allocator *allocator) {
  auto deleter = Deleter<void>(allocator, size);
  return Buffer(type, Ptr<void>(ptr, std::move(deleter)), size);
}
} // namespace sinen