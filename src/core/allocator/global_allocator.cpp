#include "global_allocator.hpp"
#include "allocator.hpp"
#include "core/allocator/allocator.hpp"
#include "tlsf_allocator.hpp"

#include <core/def/macro.hpp>
#include <core/def/types.hpp>
#include <core/logger/log.hpp>
#include <cstdlib>
#include <memory_resource>
#include <mutex>

namespace sinen {
namespace {
std::once_flag gAllocatorOnce;
std::mutex gReleaseMutex;
std::pmr::synchronized_pool_resource *gAllocator = nullptr;
TLSFAllocator *gTlsfAllocator = nullptr;
#ifdef SINEN_PLATFORM_EMSCRIPTEN
constexpr Size allocatorSize = 128 * 1024 * 1024;
#else
constexpr Size allocatorSize = 0x90000000;
#endif
} // namespace

Allocator *GlobalAllocator::get() {
  std::call_once(gAllocatorOnce, [] {
    gTlsfAllocator = new TLSFAllocator(allocatorSize);
    gAllocator = new std::pmr::synchronized_pool_resource(gTlsfAllocator);
  });

  if (!gAllocator) {
    Log::critical("Failed to application memory allocation.");
    std::exit(-1);
  }
  return gAllocator;
}

void GlobalAllocator::release() {
  std::lock_guard lock(gReleaseMutex);
  delete gAllocator;
  gAllocator = nullptr;
  delete gTlsfAllocator;
  gTlsfAllocator = nullptr;
}
} // namespace sinen
