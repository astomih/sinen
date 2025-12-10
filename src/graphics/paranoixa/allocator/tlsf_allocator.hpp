#ifndef PARANOIXA_TLSF_ALLOCATOR_HPP
#define PARANOIXA_TLSF_ALLOCATOR_HPP
#include <graphics/paranoixa/paranoixa.hpp>
#include <memory_resource>

namespace paranoixa {
class TLSFAllocator : public Allocator {
public:
  TLSFAllocator(const std::size_t &size);
  ~TLSFAllocator() override;
  void *do_allocate(std::size_t bytes, std::size_t alignment) override;
  void do_deallocate(void *p, std::size_t size, std::size_t alignment) override;
  bool
  do_is_equal(const std::pmr::memory_resource &other) const noexcept override {
    return this == &other;
  }

private:
  void *mem;
  void *tlsf;
};

} // namespace paranoixa
#endif // PARANOIXA_TLSF_ALLOCATOR_HPP