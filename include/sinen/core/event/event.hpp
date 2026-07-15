#ifndef SINEN_EVENT_SYSTEM_HPP
#define SINEN_EVENT_SYSTEM_HPP

#include <cstdint>

namespace sinen {
class Event {
public:
  enum class Type {
    Unknown,
    Quit,
    DidEnterBackground,
    WillEnterForeground,
    WindowResized,
    MouseWheel,
    KeyDown,
    TextInput,
  };

  struct MouseWheelData {
    float x;
    float y;
  };

  virtual ~Event() = default;

  static void quit() { setQuit(true); }
  static void setQuit(bool quit);
  static bool isQuit();
  static bool isPaused();

  virtual void processEvent() = 0;
  virtual Type type() const = 0;
  virtual MouseWheelData mouseWheel() const = 0;
  virtual std::uint32_t keyCode() const = 0;

protected:
  Event() = default;
  static void setPaused(bool paused);

private:
  static bool quitRequested;
  static bool paused;
};
} // namespace sinen

#endif
