#ifndef SINEN_GLOBAL_ALLOCATOR_HPP
#define SINEN_GLOBAL_ALLOCATOR_HPP
#include "allocator.hpp"

namespace sinen {
class GlobalAllocator {
public:
  static void release();
  static Allocator *get();
};
} // namespace sinen

#endif // !SINEN_GLOBAL_ALLOCATOR_HPP