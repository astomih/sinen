#include <allocator/stack_allocator.hpp>
namespace sinen {
stack_allocator::stack_allocator(const std::size_t &stack_size) {
  this->marker = (marker_t)std::malloc(stack_size);
  this->current_marker = this->marker;
}
stack_allocator::~stack_allocator() { free((void *)this->marker); }
void *stack_allocator::allocate(const std::size_t &size) {
  marker_t temp = this->current_marker;
  this->current_marker += size;
  return (void *)temp;
}
stack_allocator::marker_t stack_allocator::get_marker() {
  return this->current_marker;
}
void stack_allocator::free_to_marker(marker_t marker) {
  this->current_marker = marker;
}
void stack_allocator::clear() { this->current_marker = this->marker; }
} // namespace sinen
