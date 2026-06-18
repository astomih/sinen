#ifndef SINEN_GLOBAL_THREAD_POOL_HPP
#define SINEN_GLOBAL_THREAD_POOL_HPP

#include <core/thread/thread_pool.hpp>

#include <algorithm>
#include <cstdlib>
#include <cstddef>
#include <thread>

namespace sinen {

inline std::size_t defaultThreadPoolSize() {
  if (const char *sizeText = std::getenv("SINEN_THREAD_POOL_SIZE")) {
    char *end = nullptr;
    const unsigned long size = std::strtoul(sizeText, &end, 10);
    if (end != sizeText && *end == '\0' && size > 0) {
      return static_cast<std::size_t>(size);
    }
  }

  const unsigned int hc = std::thread::hardware_concurrency();
  const std::size_t threads = (hc > 1) ? static_cast<std::size_t>(hc - 1) : 1;
  return std::clamp<std::size_t>(threads, 1, 4);
}

inline ThreadPool &globalThreadPool() {
  static ThreadPool pool(defaultThreadPoolSize());
  return pool;
}

} // namespace sinen

#endif // SINEN_GLOBAL_THREAD_POOL_HPP
