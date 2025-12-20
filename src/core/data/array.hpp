#ifndef SINEN_ARRAY_HPP
#define SINEN_ARRAY_HPP
#include <core/allocator/global_allocator.hpp>
#include <vector>

namespace sinen {
template <typename T> class Array : public std::pmr::vector<T> {
public:
  Array() : std::pmr::vector<T>(gA) {}
  Array(Allocator *pA) : std::pmr::vector<T>(pA) {}
};
} // namespace sinen

#endif