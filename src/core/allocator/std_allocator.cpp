#include "std_allocator.hpp"

namespace sinen {
StdAllocator::StdAllocator(const std::size_t &size) {}

StdAllocator::~StdAllocator() {}

void *StdAllocator::do_allocate(std::size_t bytes, std::size_t alignment) {
  return std::pmr::get_default_resource()->allocate(bytes, alignment);
}
void StdAllocator::do_deallocate(void *ptr, std::size_t size,
                                 std::size_t alignment) {
  std::pmr::get_default_resource()->deallocate(ptr, size, alignment);
}
} // namespace sinen