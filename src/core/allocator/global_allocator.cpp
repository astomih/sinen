#include "global_allocator.hpp"
#include "allocator.hpp"
#include "core/allocator/allocator.hpp"
#include "tlsf_allocator.hpp"

#include <core/def/macro.hpp>
#include <core/def/types.hpp>
#include <core/logger/log.hpp>
#include <cstdlib>
#include <limits>
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

Size configuredAllocatorSize() {
  const char *sizeMbText = std::getenv("SINEN_GLOBAL_ALLOCATOR_SIZE_MB");
  if (!sizeMbText || sizeMbText[0] == '\0') {
    return defaultAllocatorSize;
  }

  char *end = nullptr;
  const unsigned long long sizeMb = std::strtoull(sizeMbText, &end, 10);
  constexpr unsigned long long maxSizeMb =
      std::numeric_limits<Size>::max() / bytesPerMiB;
  if (end == sizeMbText || *end != '\0' || sizeMb == 0 ||
      sizeMb > maxSizeMb) {
    return defaultAllocatorSize;
  }

  return static_cast<Size>(sizeMb) * bytesPerMiB;
}
} // namespace

Allocator *GlobalAllocator::get() {
  std::call_once(gAllocatorOnce, [] {
    gTlsfAllocator = new TLSFAllocator(configuredAllocatorSize());
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
