#include "buffer.hpp"
#include <cstddef>

namespace sinen {
Buffer::Buffer(const BufferType &type, Ptr<void> data, size_t size)
    : _size(size), _type(type), _data(data) {}
int Buffer::size() const { return _size; }
BufferType Buffer::type() const { return _type; }
void *Buffer::data() const { return _data.get(); }
} // namespace sinen