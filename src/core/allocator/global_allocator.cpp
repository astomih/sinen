#include "global_allocator.hpp"
#include "allocator.hpp"
#include "tlsf_allocator.hpp"

#include <core/def/macro.hpp>
#include <core/logger/logger.hpp>
#include <memory_resource>
namespace sinen {
GlobalAllocator::GlobalAllocator() {}
Allocator *GlobalAllocator::get() {
  if (pA)
    return pA;
  size_t allocatorSize = 0x1FFFFFF;
#ifndef SINEN_PLATFORM_EMSCRIPTEN
  pA = new TLSFAllocator(allocatorSize);
#else
  pA = new std::pmr::synchronized_pool_resource();
#endif
  if (!pA) {
    Logger::critical("Failed to application memory allocation.");
    std::exit(-1);
  }
  return pA;
}
Allocator *GlobalAllocator::pA = nullptr;
} // namespace sinen