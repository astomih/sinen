#include <core/time/time.hpp>
#include <core/time/timer.hpp>
namespace sinen {
Timer::Timer() : m_start(0.f), m_time(0.f), m_is_started(false) {}
Timer::Timer(float time) : m_start(0.f), m_time(0.f), m_is_started(false) {
  set_time(time);
}
Timer::~Timer() = default;
void Timer::start() {
  m_start = Time::milli();
  m_is_started = true;
}
void Timer::set_time(float milliSecond) { this->m_time = milliSecond; }
bool Timer::check() {
  return !m_is_started ? false : m_time <= Time::milli() - m_start;
}
void Timer::stop() { m_is_started = false; }
} // namespace sinen
