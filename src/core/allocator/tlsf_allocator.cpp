#include "tlsf_allocator.hpp"

#include <cassert>

#include <core/def/assert.hpp>
#include <tlsf.h>

namespace sinen {

TLSFAllocator::TLSFAllocator(const std::size_t &size)
    : mem(nullptr), tlsf(nullptr) {
  mem = malloc(size);
  tlsf = tlsf_create_with_pool(mem, size);
}

TLSFAllocator::~TLSFAllocator() {
  tlsf_remove_pool(tlsf, tlsf_get_pool(tlsf));
  tlsf_destroy(tlsf);
}

void *TLSFAllocator::do_allocate(std::size_t bytes, std::size_t alignment) {
  void *ptr = tlsf_memalign(tlsf, alignment, bytes);
  SN_ASSERT(ptr);
  return ptr;
}

void TLSFAllocator::do_deallocate(void *ptr, std::size_t size,
                                  std::size_t alignment) {
  tlsf_free(tlsf, ptr);
}

} // namespace sinen