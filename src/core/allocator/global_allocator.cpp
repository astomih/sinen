#include "global_allocator.hpp"
#include "allocator.hpp"
#include "core/allocator/allocator.hpp"
#include "tlsf_allocator.hpp"

#include <core/def/macro.hpp>
#include <core/def/types.hpp>
#include <core/logger/log.hpp>
#include <memory_resource>

namespace sinen {
static std::atomic<Allocator *> pA = nullptr;
Allocator *tlsf = nullptr;
static Size allocatorSize = 0x90000000;
static Size current = 0;
Allocator *GlobalAllocator::get() {
  if (pA) {
    return pA;
  }
  tlsf = new TLSFAllocator(allocatorSize);
  pA = new std::pmr::synchronized_pool_resource(tlsf);
  if (!pA) {
    Log::critical("Failed to application memory allocation.");
    std::exit(-1);
  }
  return pA;
}
void GlobalAllocator::release() {}
} // namespace sinen