#include <core/allocator/pool_allocator.hpp>
#include <core/allocator/stack_allocator.hpp>
#include <core/allocator/tlsf_allocator.hpp>

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>

namespace {
struct PoolSlot {
  std::uintptr_t value;
};

bool isAligned(void *ptr, std::size_t alignment) {
  return reinterpret_cast<std::uintptr_t>(ptr) % alignment == 0;
}
} // namespace

TEST(PoolAllocatorTest, TracksRemainingCapacityAndExhaustion) {
  sinen::PoolAllocator<PoolSlot, 3> allocator;

  EXPECT_EQ(allocator.max(), 3u);
  EXPECT_EQ(allocator.remain(), 3u);

  PoolSlot *first = allocator.allocate();
  PoolSlot *second = allocator.allocate();
  PoolSlot *third = allocator.allocate();

  EXPECT_NE(first, nullptr);
  EXPECT_NE(second, nullptr);
  EXPECT_NE(third, nullptr);
  EXPECT_NE(first, second);
  EXPECT_NE(first, third);
  EXPECT_NE(second, third);
  EXPECT_EQ(allocator.remain(), 0u);
  EXPECT_EQ(allocator.allocate(), nullptr);

  allocator.deallocate(nullptr);
  EXPECT_EQ(allocator.remain(), 0u);

  allocator.deallocate(second);
  EXPECT_EQ(allocator.remain(), 1u);
  PoolSlot *reused = allocator.allocate();
  EXPECT_EQ(reused, second);
  EXPECT_EQ(allocator.remain(), 0u);

  allocator.deallocate(reused);
  allocator.deallocate(third);
  allocator.deallocate(first);
}

TEST(StackAllocatorTest, MarkersMoveForwardAndCanRewind) {
  sinen::StackAllocator allocator(128);

  const auto initial_marker = allocator.getMaker();
  void *first = allocator.allocate(16);
  const auto after_first = allocator.getMaker();
  void *second = allocator.allocate(8);

  EXPECT_EQ(first, reinterpret_cast<void *>(initial_marker));
  EXPECT_EQ(after_first, initial_marker + 16);
  EXPECT_EQ(second, reinterpret_cast<void *>(after_first));
  EXPECT_EQ(allocator.getMaker(), after_first + 8);

  allocator.freeToMaker(after_first);
  EXPECT_EQ(allocator.getMaker(), after_first);
  EXPECT_EQ(allocator.allocate(8), second);

  allocator.clear();
  EXPECT_EQ(allocator.getMaker(), initial_marker);
  EXPECT_EQ(allocator.allocate(4), first);
}

TEST(TLSFAllocatorTest, AllocatesAlignedBlocksAndReusesFreedMemory) {
  sinen::TLSFAllocator allocator(64 * 1024);

  constexpr std::size_t kSize = 128;
  constexpr std::size_t kAlignment = 32;
  void *first = allocator.allocate(kSize, kAlignment);

  ASSERT_NE(first, nullptr);
  EXPECT_TRUE(isAligned(first, kAlignment));

  allocator.deallocate(first, kSize, kAlignment);

  void *second = allocator.allocate(kSize, kAlignment);
  ASSERT_NE(second, nullptr);
  EXPECT_TRUE(isAligned(second, kAlignment));

  allocator.deallocate(second, kSize, kAlignment);
}
