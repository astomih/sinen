#ifndef SINEN_TLSF_ALLOCATOR_HPP
#define SINEN_TLSF_ALLOCATOR_HPP
#include "allocator.hpp"

namespace sinen {
class TLSFAllocator : public Allocator {
public:
  TLSFAllocator(const std::size_t &size);
  ~TLSFAllocator() override;
  void *do_allocate(std::size_t bytes, std::size_t alignment) override;
  void do_deallocate(void *p, std::size_t size, std::size_t alignment) override;
  bool do_is_equal(const Allocator &other) const noexcept override {
    return this == &other;
  }

private:
  void *mem;
  void *tlsf;
};

} // namespace sinen
#endif // PARANOIXA_TLSF_ALLOCATOR_HPP