#ifndef SINEN_GRID_INL
#define SINEN_GRID_INL
#include <algorithm>
namespace sinen {
template <class T, class Allocator>
inline grid<T, Allocator>::grid(std::size_t width, std::size_t height)
    : m_width(width), m_data(width * height) {}
template <class T, class Allocator>
inline grid<T, Allocator>::grid(std::size_t width, std::size_t height,
                                const T &value)
    : m_width(width), m_data(width * height, value) {}
template <class T, class Allocator>
inline grid<T, Allocator>::grid(
    std::initializer_list<std::initializer_list<T>> list)
    : grid(std::max_element(
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
template <class T, class Allocator>
inline grid<T, Allocator>::grid() : m_width(0), m_data() {}
template <class T, class Allocator>
inline std::size_t grid<T, Allocator>::width() const {
  return m_width;
}
template <class T, class Allocator>
inline std::size_t grid<T, Allocator>::height() const {
  if (m_width == 0) {
    return 0;
  }
  return size() / m_width;
}

/*
  Column
*/

template <class T, class Allocator>
inline void grid<T, Allocator>::push_column() {
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
#include <iostream>
template <class T, class Allocator>
inline void grid<T, Allocator>::push_column(const T &value) {
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
template <class T, class Allocator>
inline void grid<T, Allocator>::pop_column() {
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
template <class T, class Allocator> inline void grid<T, Allocator>::push_row() {
  if (width() == 0) {
    m_data.push_back(T{});
    m_width++;
    return;
  }
  m_data.resize(m_data.size() + width());
}
template <class T, class Allocator>
inline void grid<T, Allocator>::push_row(const T &value) {
  if (width() == 0) {
    m_data.push_back(value);
    m_width++;
    return;
  }
  m_data.resize(m_data.size() + width(), value);
}
template <class T, class Allocator> inline void grid<T, Allocator>::pop_row() {
  m_data.resize(m_data.size() - width());
}

template <class T, class Allocator>
inline void grid<T, Allocator>::resize(const std::size_t &w,
                                       const std::size_t &h) {
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
      pop_column();
    }
  } else if (w > width) {
    for (std::size_t i = 0; i < w - width; i++) {
      push_column();
    }
  }
  auto height = this->height();
  if (h < height) {
    for (std::size_t i = 0; i < height - h; i++) {
      pop_row();
    }
  } else if (h > height) {
    for (std::size_t i = 0; i < h - height; i++) {
      push_row();
    }
  }
}

} // namespace sinen

#endif // SINEN_GRID_INL