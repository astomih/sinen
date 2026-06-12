#include "tlsf_allocator.hpp"
#include <core/def/assert.hpp>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <tlsf.h>

namespace sinen {

TLSFAllocator::TLSFAllocator(const std::size_t &size)
    : mem(nullptr), tlsf(nullptr) {
  mem = malloc(size);
  SN_ASSERT(mem);
  tlsf = tlsf_create_with_pool(mem, size);
  SN_ASSERT(tlsf);
}

TLSFAllocator::~TLSFAllocator() {
  if (tlsf) {
    tlsf_remove_pool(tlsf, tlsf_get_pool(tlsf));
    tlsf_destroy(tlsf);
    tlsf = nullptr;
  }
  free(mem);
  mem = nullptr;
}

void *TLSFAllocator::do_allocate(std::size_t bytes, std::size_t alignment) {
  const std::size_t effectiveAlignment =
      std::max(alignment, alignof(std::max_align_t));
  void *ptr = effectiveAlignment <= alignof(std::max_align_t)
                  ? tlsf_malloc(tlsf, bytes)
                  : tlsf_memalign(tlsf, effectiveAlignment, bytes);
  SN_ASSERT(ptr);
  return ptr;
}

void TLSFAllocator::do_deallocate(void *ptr, std::size_t size,
                                  std::size_t alignment) {
  tlsf_free(tlsf, ptr);
}

} // namespace sinen
