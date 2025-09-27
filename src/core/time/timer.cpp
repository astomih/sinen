#include <core/time/time.hpp>
#include <core/time/timer.hpp>
namespace sinen {
Timer::Timer() : startTime(0.f), currentTime(0.f), isStartedTimer(false) {}
Timer::Timer(float time) : startTime(0.f), currentTime(0.f), isStartedTimer(false) {
  setTime(time);
}
Timer::~Timer() = default;
void Timer::start() {
  startTime = Time::milli();
  isStartedTimer = true;
}
void Timer::setTime(float milliSecond) { this->currentTime = milliSecond; }
bool Timer::check() {
  return !isStartedTimer ? false : currentTime <= Time::milli() - startTime;
}
void Timer::stop() { isStartedTimer = false; }
} // namespace sinen
