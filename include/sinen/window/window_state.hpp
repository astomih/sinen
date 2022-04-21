#pragma once
namespace nen {
enum class window_state {
  SHOWN = 1,
  HIDDEN,
  EXPOSED,

  MOVED,

  RESIZED,
  SIZE_CHANGED,

  MINIMIZED,
  MAXIMIZED,
  RESTORED,

  ENTER,
  LEAVE,
  FOCUS_GAINED,
  FOCUS_LOST,
  CLOSE,
  TAKE_FOCUS,
  HIT_TEST
};
}