#ifndef SINEN_TIME_HPP
#define SINEN_TIME_HPP
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

  static float deltaTime();

  static void update();

private:
  inline static float delta = 0.f;
  inline static float prev = 0.f;
};
} // namespace sinen

#endif // SINEN_TIME_HPP
