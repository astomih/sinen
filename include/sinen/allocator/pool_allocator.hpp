#ifndef POOL_ALLOCATOR_HPP
#define POOL_ALLOCATOR_HPP
#include <cstddef>
#include <cstdlib>
/**
 * @brief Simple pool allocator
 *
 * @tparam T type of pool_allocator
 * @tparam MAXSIZE max size of pool_allocator
 */
template <class T, std::size_t MAXSIZE = 128> class pool_allocator {
public:
  /**
   * @brief Construct a new memory allocator object
   *
   */
  pool_allocator() {
    this->buffer = reinterpret_cast<element_type *>(std::malloc(sizeof(T)));
    element_type *current = this->buffer;
    for (std::size_t i = 0; i < MAXSIZE; i++) {
      element_type *next =
          reinterpret_cast<element_type *>(std::malloc(sizeof(T)));
      if (current) {
        current->next = next;
        current = next;
      }
    }
    current->next = nullptr;
  }
  /**
   * @brief Construct a new pool allocator object
   *
   * @param max_size Max size of pool
   */
  pool_allocator(const std::size_t &max_size) {
    this->buffer = reinterpret_cast<element_type *>(std::malloc(sizeof(T)));
    element_type *current = this->buffer;
    for (std::size_t i = 0; i < max_size; i++) {
      element_type *next =
          reinterpret_cast<element_type *>(std::malloc(sizeof(T)));
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
  ~pool_allocator() {
    while (true) {
      if (!this->buffer->next) {
        break;
      }
      T *ptr = this->allocate();
      std::free(ptr);
    }
    std::free(this->buffer);
  }
  /**
   * @brief Allocate new memory
   *
   * @return T* pointer of memory
   */
  T *allocate() {
    if (!this->buffer->next) {
      return nullptr;
    }
    T *ret = reinterpret_cast<T *>(this->buffer);
    this->buffer = this->buffer->next;
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
    eaddr->next = this->buffer;
    this->buffer = eaddr;
  }

private:
  union element_type {
    element_type() = default;
    ~element_type() = default;
    T v;
    element_type *next;
  };
  element_type *buffer;
};

#endif // !POOL_ALLOCATOR_HPP