#ifndef SINEN_GRID_HPP
#define SINEN_GRID_HPP
#include <iostream>
#include <stdexcept>
#include <vector>

namespace sinen {
/**
 * @brief Dynamic 2D Array
 *
 * @tparam T Type of grid
 * @tparam Allocator allocator
 */
template <class T, class Allocator = std::allocator<T>> class grid {
public:
  class const_iterator;
  class iterator;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  /**
   * @brief Construct a new grid object
   *
   * @param width  Width of grid
   * @param height  Height of grid
   */
  grid(std::size_t width, std::size_t height);
  /**
   * @brief Construct a new grid object
   *
   * @param width Width of grid
   * @param height Height of grid
   * @param value Initial value of grid
   */
  grid(std::size_t width, std::size_t height, const T &value);
  /**
   * @brief Builds a grid from an initializer list
   *
   * @param list An initializer_list
   */
  grid(std::initializer_list<std::initializer_list<T>> list);
  /**
   * @brief Construct a new grid object
   *
   */
  grid();
  /**
   * @brief Destroy the grid object
   *
   */
  ~grid() = default;

  /**
   * @brief Get the width of grid
   *
   * @return std::size_t Width of grid
   */
  inline std::size_t width() const;
  /**
   * @brief Get the height of grid
   *
   * @return std::size_t Height of grid
   */
  inline std::size_t height() const;
  /**
   * @brief Push a column to the right
   *
   */
  void push_column();
  /**
   * @brief Push a column to the right
   *
   * @param value Value to push
   */
  void push_column(const T &value);
  /**
   * @brief Pop column from the right
   *
   */
  void pop_column();
  /**
   * @brief Push row to the bottom
   *
   */
  void push_row();
  /**
   * @brief Push row to the bottom
   *
   * @param value Value to push
   */
  void push_row(const T &value);
  /**
   * @brief Pop row from the bottom
   *
   */
  void pop_row();
  /**
   * @brief Get the size of grid
   *
   * @return std::size_t Size of grid
   */
  inline std::size_t size() const noexcept { return m_data.size(); }
  /**
   * @brief Erases all elements from the grid
   *
   */
  inline void clear() noexcept {
    m_data.clear();
    m_width = 0;
  }
  /**
   * @brief Resize the grid
   *
   * @param w New width
   * @param h New Height
   */
  void resize(const std::size_t &w, const std::size_t &h);
  /**
   * @brief Capacity of grid
   *
   * @return std::size_t Capacity
   */
  std::size_t capacity() const noexcept { return m_data.capacity(); }
  /**
   * @brief Is grid empty
   *
   * @return true Empty
   * @return false Not empty
   */
  bool empty() const noexcept { return m_data.empty(); }
  /**
   * @brief Safely obtain the specified object
   *
   * @param x X coordinate
   * @param y Y coordinate
   * @return T& Object
   */
  inline T &at(const std::size_t &x, const std::size_t &y) {
    if (x < 0 || y < 0 || x >= width() || y >= height()) {
      throw std::out_of_range("grid::at(): Out of range");
    }
    return m_data[y * m_width + x];
  }
  /**
   * @brief Directly access the specified object
   *
   */
  inline T *operator[](const std::size_t &height) {
    return m_data.data() + (m_width * height);
  }
  /**
   * @brief Data access
   *
   * @return T* Data of grid
   */
  inline T *data() noexcept { return m_data.data(); }
  /**
   * @brief Get the begin iterator
   *
   * @return iterator Begin iterator
   */
  iterator begin() { return iterator(m_data.begin()); }
  /**
   * @brief Get the const begin iterator
   *
   * @return const_iterator Const begin iterator
   */
  const_iterator cbegin() { return const_iterator(m_data.cbegin()); }
  const_reverse_iterator crbegin() {
    return const_reverse_iterator(m_data.crbegin());
  }
  reverse_iterator rbegin() { return reverse_iterator(m_data.rbegin()); }
  iterator end() { return iterator(m_data.end()); }
  const_iterator cend() { return const_iterator(m_data.cend()); }
  reverse_iterator rend() { return reverse_iterator(m_data.rend()); }
  const_reverse_iterator crend() {
    return const_reverse_iterator(m_data.crend());
  }
  bool operator==(const grid &other) const noexcept {
    return m_data == other.m_data;
  }
  bool operator!=(const grid &other) const noexcept {
    return m_data != other.m_data;
  }
  /**
   * @brief const_iterator class
   *
   */
  class const_iterator {
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T *;
    using reference = T &;
    using iterator_category = std::output_iterator_tag;
    const_iterator(typename std::vector<T, Allocator>::iterator itr)
        : m_itr(itr) {}

    const T &operator*() { return *m_itr; }
    const_iterator &operator++() {
      m_itr++;
      return *this;
    }
    const_iterator &operator++(int) {
      auto ret = *this;
      m_itr++;
      return ret;
    }
    const_iterator &operator--() {
      m_itr--;
      return *this;
    }
    const_iterator &operator--(int) {
      auto ret = *this;
      m_itr--;
      return ret;
    }

    const_iterator &operator=(const_iterator itr) {
      this->m_itr = itr.m_itr;
      return *this;
    }
    const_iterator &operator+=(std::size_t n) {
      m_itr += n;
      return *this;
    }
    const_iterator &operator-=(std::size_t n) {
      m_itr -= n;
      return *this;
    }

    bool operator==(const const_iterator &rhs) const {
      return m_itr == rhs.m_itr;
    }
    bool operator!=(const const_iterator &rhs) const {
      return m_itr != rhs.m_itr;
    }

  protected:
    typename std::vector<T, Allocator>::iterator m_itr;
  };
  class iterator : public const_iterator {
  public:
    iterator(typename std::vector<T, Allocator>::iterator itr)
        : const_iterator(itr) {}
    T &operator*() { return *this->m_itr; }
  };

private:
  std::size_t m_width;
  std::vector<T, Allocator> m_data;
};
} // namespace sinen

// implementation
#include "grid.inl"
#endif // !SINEN_GRID