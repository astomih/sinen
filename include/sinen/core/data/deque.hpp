#ifndef SINEN_DEQUE_HPP
#define SINEN_DEQUE_HPP
#include <deque>

namespace sinen {
template <typename T> using Deque = std::pmr::deque<T>;
}

#endif // SINEN_DEQUE_HPP