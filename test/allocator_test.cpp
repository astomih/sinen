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
        std::cout << i << "個目のメモリ確保に失敗" << std::endl;
        FAIL();
      }
      ptr[i]->x = i;
      ptr[i]->y = i + i;
    }

    {
      POINT *p;
      p = a.allocate();
      if (nullptr != p) {
        std::cout << TEST_SIZE << "個以上のメモリ確保が出来た" << std::endl;
        FAIL();
      }
    }

    // nullptr開放
    a.deallocate(nullptr);

    // 半分開放
    for (int i = 0; i < TEST_SIZE; i += 2) {
      a.deallocate(ptr[i]);
      ptr[i] = nullptr;
    }

    // メモリ破壊チェック
    for (int i = 0; i < TEST_SIZE; ++i) {
      if (ptr[i]) {
        if (ptr[i]->x != i || ptr[i]->y != i + i) {
          std::cout << "メモリが壊れています" << std::endl;
          FAIL();
        }
      }
    }
    std::cout << "success" << std::endl;
    SUCCEED();
  }
}
} // namespace pool_allocator_test