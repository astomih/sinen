#include <time/time.hpp>
#include <time/timer.hpp>

namespace nen {
timer::timer() : m_start(0.f), m_time(0.f), m_is_started(false) {}
timer::timer(float time) : m_start(0.f), m_time(0.f), m_is_started(false) {
  set_time(time);
}

timer::~timer() = default;

void timer::start() {
  m_start = time::get_ticks_as_milli_seconds();
  m_is_started = true;
}

/* 時刻を設定 */
void timer::set_time(float milliSecond) { this->m_time = milliSecond; }

/* 設定時刻を経過したか */
bool timer::check() {
  return !m_is_started ? false
                       : m_time <= time::get_ticks_as_milli_seconds() - m_start;
}

void timer::stop() { m_is_started = false; }
} // namespace nen