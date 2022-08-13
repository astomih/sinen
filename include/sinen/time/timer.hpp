#ifndef SINEN_TIME_TIMER_HPP
#define SINEN_TIME_TIMER_HPP

namespace sinen {
/**
 * @brief Timer class
 *
 */
class timer {
public:
  /**
   * @brief Construct a new timer object
   *
   */
  timer();
  /**
   * @brief Construct a new timer object
   *
   * @param time Set the time in seconds
   */
  timer(float time);
  /**
   * @brief Destroy the timer object
   *
   */
  ~timer();
  /**
   * @brief Start the timer
   *
   */
  void start();
  /**
   * @brief Set the time object
   *
   * @param milliSecond Set the time in milliseconds
   */
  void set_time(float milliseconds);
  /**
   * @brief Is the timer finished
   *
   * @return true The timer is finished
   * @return false The timer is not finished
   */
  bool check();
  /**
   * @brief Is the timer started
   *
   * @return true The timer is started
   * @return false The timer is not started
   */
  bool is_started() { return m_is_started; }
  /**
   * @brief Stop the timer
   *
   */
  void stop();

private:
  float m_start;
  float m_time;
  bool m_is_started;
};
} // namespace sinen
#endif // SINEN_TIME_TIMER_HPP
