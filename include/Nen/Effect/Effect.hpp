#pragma once
#include "../Math/Quaternion.hpp"
#include "../Math/Vector3.hpp"
#include "../Time/Timer.hpp"
#include <string>


namespace nen {
class effect {
public:
  effect(std::string_view path);
  ~effect();
  const vector3 &GetPosition() { return this->position; }
  const quaternion &GetRotation() { return this->rotate; }
  void SetPosition(const vector3 &pos) { this->position = pos; }
  void SetRotation(const quaternion &rot) { this->rotate = rot; }
  const std::u16string &GetPath() { return path; }
  int handle;

  bool isLoop() { return loop; }
  void SetLoop(bool loop, timer interval) {
    this->loop = loop;
    this->interval = interval;
  }

  timer &GetTimer() { return interval; }

  bool first = true;

  enum class State { Active, Dead };

  State state = State::Active;

private:
  vector3 position;
  quaternion rotate;
  std::u16string path;
  /*
          ループ再生させるのか？
  */
  bool loop;
  /*
   * ループの間隔
   */
  timer interval;
};
} // namespace nen