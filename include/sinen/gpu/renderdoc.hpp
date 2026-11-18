#ifndef SINEN_RENDERDOC_HPP
#define SINEN_RENDERDOC_HPP

#include <core/data/string.hpp>
#include <core/def/types.hpp>

namespace sinen {
// Call from the render thread. The capture library stays loaded until exit.
class RenderDoc {
public:
  // Attach to an injected library, or load SINEN_RENDERDOC_LIBRARY if set.
  // Must run before GPU device creation when explicitly loading the library.
  static bool initialize();
  static bool isAvailable();
  // Queue capture of the next presented frame(s); true means requested.
  static bool triggerCapture(UInt32 numFrames = 1);
  static bool isFrameCapturing();
  static bool setCaptureFilePathTemplate(const char *path);
  static UInt32 getNumCaptures();
  // Zero-based capture index. Returns an empty string if unavailable/invalid.
  static String getCaptureFilePath(UInt32 index);
};
} // namespace sinen

#endif
