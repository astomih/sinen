#ifndef SINEN_DEQUE_HPP
#define SINEN_DEQUE_HPP
#include "deque.hpp"

#include <queue>

namespace sinen {
template <typename T> using Queue = std::queue<T, std::pmr::deque<T>>;
}

#endif // SINEN_DEQUE_HPP