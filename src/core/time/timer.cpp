#include <core/time/time.hpp>
#include <core/time/timer.hpp>
namespace sinen {
Timer::Timer() : m_start(0.f), m_time(0.f), m_is_started(false) {}
Timer::Timer(float time) : m_start(0.f), m_time(0.f), m_is_started(false) {
  SetTime(time);
}
Timer::~Timer() = default;
void Timer::Start() {
  m_start = Time::Milli();
  m_is_started = true;
}
void Timer::SetTime(float milliSecond) { this->m_time = milliSecond; }
bool Timer::Check() {
  return !m_is_started ? false : m_time <= Time::Milli() - m_start;
}
void Timer::Stop() { m_is_started = false; }
} // namespace sinen
