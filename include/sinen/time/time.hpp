#ifndef NEN_TIME_HPP
#define NEN_TIME_HPP
#include <chrono>
#include <cstdint>
#include <functional>

namespace nen {
/**
 * @brief About time
 *
 */
class time {
public:
  /**
   * @brief Get time about launch app to now as seconds
   */
  static float GetTicksAsSeconds();

  /**
   * @brief Get time about launch app to now as milli seconds
   */
  static uint32_t GetTicksAsMilliSeconds();

  /**
   * @brief Get the function time object
   *
   * @param function
   * @return double time
   */
  static inline double get_function_time(std::function<void()> function) {
    std::chrono::system_clock::time_point start, end;
    start = std::chrono::system_clock::now();
    function();
    end = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(end - start)
        .count();
  }
};
} // namespace nen

#endif // NEN_TIME_HPP
