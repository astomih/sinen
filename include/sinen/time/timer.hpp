#ifndef SINEN_TIME_TIMER_HPP
#define SINEN_TIME_TIMER_HPP

namespace nen {
class timer {
public:
  timer();
  timer(float time);
  ~timer();

  void start();

  void set_time(float milliSecond);

  bool check();

  bool is_started() { return m_is_started; }

  void stop();

private:
  float m_start;
  float m_time;
  bool m_is_started;
};
} // namespace nen
#endif // SINEN_TIME_TIMER_HPP