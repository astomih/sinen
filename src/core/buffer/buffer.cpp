#include <core/buffer/buffer.hpp>
#include <core/allocator/global_allocator.hpp>
#include <cstddef>
#include <graphics/camera/camera3d.hpp>
#include <math/matrix.hpp>
#include <math/vector.hpp>

#include <cstring>
#include <iomanip>
#include <sstream>

namespace sinen {
Buffer::Buffer(const BufferType &type, Ptr<void> data, size_t size)
    : _size(size), _type(type), _data(data) {}
int Buffer::size() const { return _size; }
BufferType Buffer::type() const { return _type; }
void *Buffer::data() const { return _data.get(); }
Buffer makeBuffer(size_t size, BufferType type, Allocator *allocator) {
  constexpr size_t alignment = alignof(std::max_align_t);
  auto *ptr = allocator->allocate(size, alignment);
  auto deleter = Deleter<void>(allocator, size, alignment);
  return Buffer(type, Ptr<void>(ptr, std::move(deleter)), size);
}
Buffer makeBuffer(void *ptr, size_t size, BufferType type,
                  Allocator *allocator) {
  auto deleter = Deleter<void>(allocator, size, alignof(std::max_align_t));
  return Buffer(type, Ptr<void>(ptr, std::move(deleter)), size);
}

} // namespace sinen
