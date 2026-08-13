#ifndef SINEN_DEBUG_OVERLAY_HPP
#define SINEN_DEBUG_OVERLAY_HPP

#include <core/data/ptr.hpp>
#include <core/data/string.hpp>
#include <graphics/texture/texture.hpp>

#include <cstdint>
#include <vector>

namespace sinen {
class DebugOverlay {
public:
  void initialize();
  void shutdown();
  void update(float deltaTime, bool togglePressed);

private:
  void draw();
  void refreshTexture();

  Ptr<Texture> texture;
  std::vector<uint8_t> pixelBuffer;
  String gpuName;
  String gpuDriver;
  String platform;
  float sampleSeconds = 0.0f;
  float displayedFps = 0.0f;
  float displayedFrameMs = 0.0f;
  uint32_t sampleFrames = 0;
  int cpuCores = 0;
  int systemRamMiB = 0;
  bool visible = false;
  bool textureDirty = true;
};
} // namespace sinen

#endif // SINEN_DEBUG_OVERLAY_HPP
