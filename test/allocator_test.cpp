// std
#include <cstdint>
// gtest
#include <gtest/gtest.h>
// allocator
#include <sinen/allocator/pool_allocator.hpp>
#include <sinen/allocator/stack_allocator.hpp>
namespace pool_allocator_test {
/**
 * @brief Test the pool allocator
 * @details Test the pool allocator
 */
TEST(PoolAllocatorTest, TestPoolAllocatorConstruct) {
  { sinen::pool_allocator<std::uint64_t> pool; }
  SUCCEED();
}
TEST(PoolAllocatorTest, TestPoolAllocatorAllocate) {
  {
    struct POINT {
      int x;
      int y;
    };

    const int TEST_SIZE = 100;
    sinen::pool_allocator<POINT, TEST_SIZE> a;
    POINT *ptr[TEST_SIZE];
    for (int i = 0; i < TEST_SIZE; ++i) {
      ptr[i] = a.allocate();
      if (nullptr == ptr[i]) {
        FAIL();
      }
      ptr[i]->x = i;
      ptr[i]->y = i + i;
    }
    {
      POINT *p;
      p = a.allocate();
      if (nullptr != p) {
        FAIL();
      }
    }
    // nullptr
    a.deallocate(nullptr);
    for (int i = 0; i < TEST_SIZE; i += 2) {
      a.deallocate(ptr[i]);
      ptr[i] = nullptr;
    }
    for (int i = 0; i < TEST_SIZE; ++i) {
      if (ptr[i]) {
        if (ptr[i]->x != i || ptr[i]->y != i + i) {
          FAIL();
        }
      }
    }
    SUCCEED();
  }
}
} // namespace pool_allocator_test