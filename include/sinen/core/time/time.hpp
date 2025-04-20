#ifndef SINEN_TIME_HPP
#define SINEN_TIME_HPP
#include <chrono>
#include <cstdint>
#include <functional>

namespace sinen {
/**
 * @brief About time
 *
 */
class Time {
public:
  /**
   * @brief Get time about launch app to now as seconds
   */
  static float seconds();

  /**
   * @brief Get time about launch app to now as milli seconds
   */
  static uint32_t milli();

  /**
   * @brief Get the function time object
   *
   * @param function
   * @return double time
   */
  static inline double
  get_function_time(const std::function<void()> &function) {
    std::chrono::system_clock::time_point start, end;
    start = std::chrono::system_clock::now();
    function();
    end = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(end - start)
        .count();
  }
};
} // namespace sinen

#endif // SINEN_TIME_HPP
