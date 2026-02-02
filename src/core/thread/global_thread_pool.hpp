#ifndef SINEN_GLOBAL_THREAD_POOL_HPP
#define SINEN_GLOBAL_THREAD_POOL_HPP

#include <core/thread/thread_pool.hpp>

#include <algorithm>
#include <cstddef>
#include <thread>

namespace sinen {

inline std::size_t defaultThreadPoolSize() {
  const unsigned int hc = std::thread::hardware_concurrency();
  const std::size_t threads = (hc > 1) ? static_cast<std::size_t>(hc - 1) : 1;
  return std::max<std::size_t>(threads, 1);
}

inline ThreadPool &globalThreadPool() {
  static ThreadPool pool(defaultThreadPoolSize());
  return pool;
}

} // namespace sinen

#endif // SINEN_GLOBAL_THREAD_POOL_HPP
