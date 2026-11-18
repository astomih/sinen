#include <renderdoc_app.h>

#include <cstring>
#include <string>

#if defined(_WIN32)
#define FIXTURE_EXPORT extern "C" __declspec(dllexport)
#else
#define FIXTURE_EXPORT extern "C" __attribute__((visibility("default")))
#endif

namespace {
bool acceptAPI = false;
uint32_t frames = 0;
std::string pathTemplate;
constexpr char capturePath[] = "captures/sinen_frame42.rdc";
RENDERDOC_API_1_1_2 api{};
} // namespace

FIXTURE_EXPORT void fixtureAcceptAPI() { acceptAPI = true; }
FIXTURE_EXPORT uint32_t fixtureFrames() { return frames; }
FIXTURE_EXPORT const char *fixturePathTemplate() {
  return pathTemplate.c_str();
}

FIXTURE_EXPORT int RENDERDOC_CC RENDERDOC_GetAPI(RENDERDOC_Version version,
                                                 void **out) {
  *out = nullptr;
  if (!acceptAPI || version != eRENDERDOC_API_Version_1_1_2) {
    return 0;
  }
  api.TriggerCapture = [] { frames = 1; };
  api.TriggerMultiFrameCapture = [](uint32_t count) { frames = count; };
  api.IsFrameCapturing = []() -> uint32_t { return frames > 0; };
  api.SetCaptureFilePathTemplate = [](const char *path) {
    pathTemplate = path;
  };
  api.GetNumCaptures = []() -> uint32_t { return frames > 0 ? 1 : 0; };
  api.GetCapture = [](uint32_t index, char *path, uint32_t *length,
                      uint64_t *timestamp) -> uint32_t {
    if (index != 0 || frames == 0) {
      return 0;
    }
    if (path) {
      std::memcpy(path, capturePath, sizeof(capturePath));
    }
    if (length) {
      *length = sizeof(capturePath);
    }
    if (timestamp) {
      *timestamp = 0;
    }
    return 1;
  };
  *out = &api;
  return 1;
}
