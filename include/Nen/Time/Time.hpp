#ifndef NEN_TIME_HPP
#define NEN_TIME_HPP
#include <cstdint>

namespace nen
{
    /**
     * 時間に関するクラス
     */
    class Time
    {
    public:
        /*
         * アプリが起動してからの時間(秒)
        */
        static float GetTicksAsSeconds();

        /*
         * アプリが起動してからの時間(ミリ秒)
        */
        static uint32_t GetTicksAsMilliSeconds();
    };
}

#endif // NEN_TIME_HPP
