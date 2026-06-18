#ifndef SINEN_ARRAY_HPP
#define SINEN_ARRAY_HPP
#include <vector>

namespace sinen {
template <typename T> using Array = std::pmr::vector<T>;
} // namespace sinen

#endif