#ifndef SINEN_GRID_HPP
#define SINEN_GRID_HPP
#include <cassert>
#include <core/data/array.hpp>
#include <math/geometry/rect.hpp>

namespace sinen {
namespace internal {

/**
 * @brief Dynamic 2D Array
 *
 * @tparam T Type of grid
 */
template <class T> class Grid {
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
  Grid(std::size_t width, std::size_t height);
  /**
   * @brief Construct a new grid object
   *
   * @param width Width of grid
   * @param height Height of grid
   * @param value Initial value of grid
   */
  Grid(std::size_t width, std::size_t height, const T &value);
  /**
   * @brief Builds a grid from an initializer list
   *
   * @param list An initializer_list
   */
  Grid(std::initializer_list<std::initializer_list<T>> list);
  /**
   * @brief Construct a new grid object
   *
   */
  Grid();
  /**
   * @brief Destroy the grid object
   *
   */
  ~Grid() = default;

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
  void pushColumn();
  /**
   * @brief Push a column to the right
   *
   * @param value Value to push
   */
  void pushColumn(const T &value);
  /**
   * @brief Pop column from the right
   *
   */
  void popColumn();
  /**
   * @brief Push row to the bottom
   *
   */
  void pushRow();
  /**
   * @brief Push row to the bottom
   *
   * @param value Value to push
   */
  void pushRow(const T &value);
  /**
   * @brief Pop row from the bottom
   *
   */
  void popRow();

  inline void fill(const T &value) {
    for (auto &i : m_data) {
      i = value;
    }
  }

  inline void fillRect(const Rect &rect, const T &value) {
    for (int y = rect.y; y < rect.y + rect.height; y++) {
      for (int x = rect.x; x < rect.x + rect.width; x++) {
        m_data[y * width() + x] = value;
      }
    }
  }

  inline void setRow(int index, const T &value) {
    for (int x = 0; x < width(); x++)
      m_data[index * width() + x] = value;
  }

  inline void setColumn(int index, const T &value) {
    for (int y = 0; y < height(); y++)
      m_data[y * width() + index] = value;
  }

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
    assert(x >= 0 && y >= 0 && x < width() && y < height());
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
  bool operator==(const Grid &other) const noexcept {
    return m_data == other.m_data;
  }
  bool operator!=(const Grid &other) const noexcept {
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
    const_iterator(typename Array<T>::iterator itr) : m_itr(itr) {}

    const T &operator*() { return *m_itr; }
    const_iterator &operator++() {
      m_itr++;
      return *this;
    }
    const_iterator &operator++(int) {
      auto &ret = *this;
      m_itr++;
      return ret;
    }
    const_iterator &operator--() {
      m_itr--;
      return *this;
    }
    const_iterator &operator--(int) {
      auto &ret = *this;
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
    typename Array<T>::iterator m_itr;
  };
  class iterator : public const_iterator {
  public:
    iterator(typename Array<T>::iterator itr) : const_iterator(itr) {}
    T &operator*() { return *this->m_itr; }
  };

private:
  std::size_t m_width;
  Array<T> m_data;
};

// ---------------
// implementation
// ---------------
template <class T>
inline Grid<T>::Grid(std::size_t width, std::size_t height)
    : m_width(width), m_data(width * height) {}
template <class T>
inline Grid<T>::Grid(std::size_t width, std::size_t height, const T &value)
    : m_width(width), m_data(width * height, value) {}
template <class T>
inline Grid<T>::Grid(std::initializer_list<std::initializer_list<T>> list)
    : Grid(std::max_element(
               list.begin(), list.end(),
               [](auto &lhs, auto &rhs) { return lhs.size() < rhs.size(); })
               ->size(),
           list.size()) {
  auto dst = begin();

  for (const auto &a : list) {
    std::copy(a.begin(), a.end(), dst);
    dst += m_width;
  }
}
template <class T> inline Grid<T>::Grid() : m_width(0), m_data() {}
template <class T> inline std::size_t Grid<T>::width() const { return m_width; }
template <class T> inline std::size_t Grid<T>::height() const {
  if (m_width == 0) {
    return 0;
  }
  return size() / m_width;
}

/*
  Column
*/

template <class T> inline void Grid<T>::pushColumn() {
  if (m_width == 0) {
    m_data.push_back(T{});
    m_width++;
    return;
  }
  for (auto it = m_data.begin(); it != m_data.end(); it++) {
    it = m_data.insert(it + width(), T{});
  }
  m_width++;
}
template <class T> inline void Grid<T>::pushColumn(const T &value) {
  if (m_width == 0) {
    m_data.push_back(value);
    m_width++;
    return;
  }
  for (auto it = m_data.begin(); it != m_data.end(); it++) {
    it = m_data.insert(it + width(), value);
  }
  m_width++;
}
template <class T> inline void Grid<T>::popColumn() {
  if (m_width == 0) {
    return;
  }
  for (auto it = m_data.begin(); it != m_data.end(); it++) {
    it = --m_data.erase(it + width() - 1);
  }
  m_width--;
}

/*
  Row
*/
template <class T> inline void Grid<T>::pushRow() {
  if (width() == 0) {
    m_data.push_back(T{});
    m_width++;
    return;
  }
  m_data.resize(m_data.size() + width());
}
template <class T> inline void Grid<T>::pushRow(const T &value) {
  if (width() == 0) {
    m_data.push_back(value);
    m_width++;
    return;
  }
  m_data.resize(m_data.size() + width(), value);
}
template <class T> inline void Grid<T>::popRow() {
  m_data.resize(m_data.size() - width());
}

template <class T>
inline void Grid<T>::resize(const std::size_t &w, const std::size_t &h) {
  if (w < 2 || h < 2) {
    if (w == 0 || h == 0) {
      m_data.clear();
      m_width = 0;
      return;
    }
    if (w == 1) {
      m_data.resize(h);
      m_width = 1;
      return;
    }
    if (h == 1) {
      m_data.resize(w);
      m_width = w;
      return;
    }
  }
  auto width = this->width();
  if (w < width) {
    for (std::size_t i = 0; i < width - w; i++) {
      popColumn();
    }
  } else if (w > width) {
    for (std::size_t i = 0; i < w - width; i++) {
      pushColumn();
    }
  }
  auto height = this->height();
  if (h < height) {
    for (std::size_t i = 0; i < height - h; i++) {
      popRow();
    }
  } else if (h > height) {
    for (std::size_t i = 0; i < h - height; i++) {
      pushRow();
    }
  }
}
} // namespace internal
using Grid = internal::Grid<float>;
} // namespace sinen
#endif // !SINEN_GRID_HPP