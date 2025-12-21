#include "global_allocator.hpp"
#include "tlsf_allocator.hpp"
namespace sinen {
GlobalAllocator::GlobalAllocator() {
  size_t allocatorSize = 0x400000;

  Allocator *pA;
#ifdef _MSC_VER
  pA = new TLSFAllocator(allocatorSize);
#else
  pA = new StdAllocator(allocatorSize);
#endif

  setAllocator(pA);
}
} // namespace sinen