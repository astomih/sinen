#include <Time/Time.hpp>
#include <Time/Timer.hpp>

namespace nen
{
    Timer::Timer()
        : start(0.f), time(0.f), isStart(false)
    {
    }
    Timer::Timer(float time)
        : start(0.f), time(0.f), isStart(false)
    {
        SetTime(time);
    }

    Timer::~Timer() = default;

    /* 計測開始 */
    void Timer::Start()
    {
        start = Time::GetTicksAsMilliSeconds();
        isStart = true;
    }

    /* 時刻を設定 */
    void Timer::SetTime(float milliSecond)
    {
        this->time = milliSecond;
    }

    /* 設定時刻を経過したか */
    bool Timer::Check()
    {
        return !isStart ? false : time <= Time::GetTicksAsMilliSeconds() - start;
    }

    void Timer::Stop()
    {
        isStart = false;
    }
}