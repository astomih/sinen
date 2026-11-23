#include <core/allocator/tlsf_allocator.hpp>
#include <core/def/assert.hpp>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <memory_resource>
#include <new>
#include <tlsf.h>

namespace sinen {

TLSFAllocator::TLSFAllocator(const std::size_t &size, bool overflow)
    : mem(nullptr), tlsf(nullptr), poolSize(size), allowOverflow(overflow) {
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
  std::lock_guard lock(mtx);
  const std::size_t effectiveAlignment =
      std::max(alignment, alignof(std::max_align_t));
  void *ptr = effectiveAlignment <= alignof(std::max_align_t)
                  ? tlsf_malloc(tlsf, bytes)
                  : tlsf_memalign(tlsf, effectiveAlignment, bytes);
  if (!ptr) {
    if (allowOverflow)
      return std::pmr::new_delete_resource()->allocate(bytes,
                                                       effectiveAlignment);
    // memory_resource must throw on exhaustion. Logging an assertion here can
    // re-enter the synchronized pool while its upstream allocation is locked.
    throw std::bad_alloc();
  }
  return ptr;
}

void TLSFAllocator::do_deallocate(void *ptr, std::size_t size,
                                  std::size_t alignment) {
  std::lock_guard lock(mtx);
  const auto address = reinterpret_cast<std::uintptr_t>(ptr);
  const auto beginning = reinterpret_cast<std::uintptr_t>(mem);
  if (allowOverflow &&
      (address < beginning || address - beginning >= poolSize)) {
    std::pmr::new_delete_resource()->deallocate(
        ptr, size, std::max(alignment, alignof(std::max_align_t)));
    return;
  }
  tlsf_free(tlsf, ptr);
}

} // namespace sinen
