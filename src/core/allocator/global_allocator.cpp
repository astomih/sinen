#include <core/allocator/allocator.hpp>
#include <core/allocator/global_allocator.hpp>
#include <core/allocator/tlsf_allocator.hpp>
#include <core/def/assert.hpp>
#include <core/def/macro.hpp>
#include <core/def/types.hpp>
#include <core/logger/log.hpp>

#include <memory_resource>
#include <mutex>

namespace sinen {
namespace {
std::once_flag gAllocatorOnce;
std::mutex gReleaseMutex;
std::pmr::synchronized_pool_resource *gAllocator = nullptr;
TLSFAllocator *gTlsfAllocator = nullptr;
constexpr Size bytesPerMiB = 1024 * 1024;
#ifdef SINEN_PLATFORM_EMSCRIPTEN
constexpr Size defaultAllocatorSize = 128 * bytesPerMiB;
#else
constexpr Size defaultAllocatorSize = 256 * bytesPerMiB;
#endif

} // namespace

Allocator *GlobalAllocator::get() {
  std::call_once(gAllocatorOnce, [] {
    gTlsfAllocator = new TLSFAllocator(defaultAllocatorSize);
    gAllocator = new std::pmr::synchronized_pool_resource(gTlsfAllocator);
  });

  if (!gAllocator) {
    Log::critical("Failed to application memory allocation.");
    return nullptr;
  }
  return gAllocator;
}

void GlobalAllocator::release() {
  std::lock_guard lock(gReleaseMutex);
  SN_ASSERT(gAllocator);
  delete gAllocator;
  gAllocator = nullptr;
  SN_ASSERT(gTlsfAllocator);
  delete gTlsfAllocator;
  gTlsfAllocator = nullptr;
}
} // namespace sinen
