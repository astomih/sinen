#ifndef SINEN_STD_ALLOCATOR_HPP
#define SINEN_STD_ALLOCATOR_HPP
#include "allocator.hpp"

namespace sinen {
class StdAllocator : public Allocator {
public:
  StdAllocator(const std::size_t &size);
  ~StdAllocator() override;
  void *do_allocate(std::size_t bytes, std::size_t alignment) override;
  void do_deallocate(void *p, std::size_t size, std::size_t alignment) override;

  bool do_is_equal(const Allocator &other) const noexcept override {
    return this == &other;
  }

private:
};

} // namespace sinen
#endif