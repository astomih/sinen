// internal
#include <core/allocator/stack_allocator.hpp>

namespace sinen {
StackAllocator::StackAllocator(const std::size_t &stack_size) {
  this->marker = (marker_t)std::malloc(stack_size);
  this->current_marker = this->marker;
}
StackAllocator::~StackAllocator() { free((void *)this->marker); }
void *StackAllocator::allocate(const std::size_t &size) {
  marker_t temp = this->current_marker;
  this->current_marker += size;
  return (void *)temp;
}
StackAllocator::marker_t StackAllocator::getMaker() {
  return this->current_marker;
}
void StackAllocator::freeToMaker(marker_t marker) {
  this->current_marker = marker;
}
void StackAllocator::clear() { this->current_marker = this->marker; }
} // namespace sinen
