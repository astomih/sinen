#include <Time/Time.hpp>
#include <Time/Timer.hpp>

namespace nen {
timer::timer() : start(0.f), time(0.f), isStart(false) {}
timer::timer(float time) : start(0.f), time(0.f), isStart(false) {
  SetTime(time);
}

timer::~timer() = default;

/* 計測開始 */
void timer::Start() {
  start = time::GetTicksAsMilliSeconds();
  isStart = true;
}

/* 時刻を設定 */
void timer::SetTime(float milliSecond) { this->time = milliSecond; }

/* 設定時刻を経過したか */
bool timer::Check() {
  return !isStart ? false : time <= time::GetTicksAsMilliSeconds() - start;
}

void timer::Stop() { isStart = false; }
} // namespace nen