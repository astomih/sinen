#ifndef SINEN_TIME_TIMER_HPP
#define SINEN_TIME_TIMER_HPP

namespace sinen {
/**
 * @brief Timer class
 *
 */
class Timer {
public:
  /**
   * @brief Construct a new timer object
   *
   */
  Timer();
  /**
   * @brief Construct a new timer object
   *
   * @param time Set the time in seconds
   */
  Timer(float time);
  /**
   * @brief Destroy the timer object
   *
   */
  ~Timer();
  /**
   * @brief Start the timer
   *
   */
  void Start();
  /**
   * @brief Set the time object
   *
   * @param milliSecond Set the time in milliseconds
   */
  void SetTime(float milliseconds);
  /**
   * @brief Is the timer finished
   *
   * @return true The timer is finished
   * @return false The timer is not finished
   */
  bool Check();
  /**
   * @brief Is the timer started
   *
   * @return true The timer is started
   * @return false The timer is not started
   */
  bool IsStarted() { return m_is_started; }
  /**
   * @brief Stop the timer
   *
   */
  void Stop();

private:
  float m_start;
  float m_time;
  bool m_is_started;
};
} // namespace sinen
#endif // SINEN_TIME_TIMER_HPP
