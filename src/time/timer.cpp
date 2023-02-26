#include <time/time.hpp>
#include <time/timer.hpp>
namespace sinen {
timer::timer() : m_start(0.f), m_time(0.f), m_is_started(false) {}
timer::timer(float time) : m_start(0.f), m_time(0.f), m_is_started(false) {
  set_time(time);
}
timer::~timer() = default;
void timer::start() {
  m_start = time::milli();
  m_is_started = true;
}
void timer::set_time(float milliSecond) { this->m_time = milliSecond; }
bool timer::check() {
  return !m_is_started ? false : m_time <= time::milli() - m_start;
}
void timer::stop() { m_is_started = false; }
} // namespace sinen
