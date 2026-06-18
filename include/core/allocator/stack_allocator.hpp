#ifndef SINEN_STACK_ALLOCATOR_HPP
#define SINEN_STACK_ALLOCATOR_HPP
#include <cstdint>
#include <cstdlib>
namespace sinen {
/**
 * @brief Simple stack allocator
 *
 */
class StackAllocator {
public:
  /**
   * @brief Construct a new stack allocator object
   *
   * @param stack_size Stack size
   */
  explicit StackAllocator(const std::size_t &stack_size);
  /**
   * @brief Destroy the stack allocator object
   *
   */
  virtual ~StackAllocator();
  /**
   * @brief Allocate memory
   *
   * @param size Allocate size
   * @return void* ptr
   */
  void *allocate(const std::size_t &size);
  using marker_t = std::intptr_t;
  /**
   * @brief Get the marker
   *
   * @return marker_t
   */
  marker_t getMaker();
  /**
   * @brief Free to marker
   *
   * @param marker marker
   */
  void freeToMaker(marker_t marker);
  /**
   * @brief Clear the stack
   *
   */
  void clear();

private:
  marker_t marker;
  marker_t current_marker;
};
} // namespace sinen
#endif // SINEN_STACK_ALLOCATOR_HPP