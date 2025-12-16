#ifndef POOL_ALLOCATOR_HPP
#define POOL_ALLOCATOR_HPP
#include <cstddef>
#include <cstdlib>
namespace sinen {
/**
 * @brief Simple pool allocator
 *
 * @tparam T type of pool_allocator
 * @tparam MAXSIZE max size of pool_allocator
 */
template <class T, std::size_t MAXSIZE = 128> class PoolAllocator {
public:
  /**
   * @brief Construct a new memory allocator object
   *
   */
  PoolAllocator() {
    this->head = reinterpret_cast<element_type *>(
        std::malloc(sizeof(T) * (MAXSIZE + 1)));
    this->free_list = this->head;
    element_type *current = this->head;
    for (std::size_t i = 0; i < MAXSIZE; i++) {
      element_type *next = reinterpret_cast<element_type *>(current + 1);
      if (current) {
        current->next = next;
        current = next;
      }
    }
    current->next = nullptr;
  }
  /**
   * @brief Destroy the memory allocator object
   *
   */
  ~PoolAllocator() { std::free(this->head); }
  /**
   * @brief Allocate new memory
   *
   * @return T* pointer of memory
   */
  T *allocate() {
    if (!this->free_list->next) {
      return nullptr;
    }
    T *ret = reinterpret_cast<T *>(this->free_list);
    this->free_list = this->free_list->next;
    return ret;
  }
  /**
   * @brief Deallocate memory
   *
   * @param addr allocated memory
   */
  void deallocate(T *addr) {
    if (!addr) {
      return;
    }
    element_type *eaddr = reinterpret_cast<element_type *>(addr);
    eaddr->next = this->free_list;
    this->free_list = eaddr;
  }
  /**
   * @brief Get the max size pool
   *
   * @return const std::size_t
   */
  const std::size_t max() const { return MAXSIZE; }
  /**
   * @brief Get the size of pool
   *
   * @return const std::size_t
   */
  const std::size_t remain() const {
    std::size_t ret = 0;
    element_type *current = this->free_list;
    while (current->next) {
      ret++;
      current = current->next;
    }
    return ret;
  }

private:
  union element_type {
    element_type() = default;
    ~element_type() = default;
    T v;
    element_type *next;
  };
  element_type *head;
  element_type *free_list;
};
} // namespace sinen
#endif // !POOL_ALLOCATOR_HPP