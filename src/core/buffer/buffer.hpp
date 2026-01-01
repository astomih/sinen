#ifndef SINEN_BUFFER_HPP
#define SINEN_BUFFER_HPP
#include <core/data/ptr.hpp>
#include <cstddef>
namespace sinen {
enum class BufferType {
  Binary,
  String,
};
class Buffer {
public:
  Buffer(const BufferType &type, Ptr<void> data, size_t size);
  int size() const;
  BufferType type() const;
  void *data() const;

private:
  size_t _size;
  BufferType _type;
  Ptr<void> _data;
};
} // namespace sinen
#endif