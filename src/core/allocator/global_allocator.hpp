#ifndef SINEN_GLOBAL_ALLOCATOR_HPP
#define SINEN_GLOBAL_ALLOCATOR_HPP
#include "SDL3/SDL_thread.h"
#include "allocator.hpp"

namespace sinen {
class GlobalAllocator : public Allocator {
public:
  GlobalAllocator();
  ~GlobalAllocator() override { pA = nullptr; }
  void setAllocator(Allocator *allocator) { pA = allocator; }
  void *do_allocate(std::size_t bytes, std::size_t alignment) override {
    return pA->allocate(bytes, alignment);
  }
  void do_deallocate(void *p, std::size_t size,
                     std::size_t alignment) override {
    pA->deallocate(p, size, alignment);
  }
  bool do_is_equal(const Allocator &other) const noexcept override {
    return this == &other;
  }

private:
  Allocator *pA;
};
inline static GlobalAllocator *gA = new GlobalAllocator();
} // namespace sinen

#endif // !SINEN_GLOBAL_ALLOCATOR_HPP