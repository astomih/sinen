#pragma once

namespace nen {
class timer {
public:
  timer();
  timer(float time);
  ~timer();

  /* 計測開始 */
  void Start();

  /* 時刻を設定 */
  void SetTime(float milliSecond);

  /* 設定時刻を経過したか */
  bool Check();

  /* 計測中か */
  bool isStarted() { return isStart; }

  void Stop();

private:
  /* 計測開始時の時刻 */
  float start;
  /* 設定時刻 */
  float time;
  /* 計測中かどうか */
  bool isStart;
};
} // namespace nen