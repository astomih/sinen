#include <gpu/renderdoc.hpp>

#include <SDL3/SDL.h>
#include <gtest/gtest.h>

// One test keeps the process-wide API lifecycle in a deterministic order.
TEST(RenderDocTests, OptionalLibraryNegotiationAndCaptureForwarding) {
  using sinen::RenderDoc;
  auto *environment = SDL_GetEnvironment();
  ASSERT_NE(environment, nullptr);
  const char *oldPath =
      SDL_GetEnvironmentVariable(environment, "SINEN_RENDERDOC_LIBRARY");
  const std::string savedPath = oldPath ? oldPath : "";
  struct RestoreEnvironment {
    SDL_Environment *environment;
    bool hadPath;
    std::string path;
    ~RestoreEnvironment() {
      if (hadPath) {
        SDL_SetEnvironmentVariable(environment, "SINEN_RENDERDOC_LIBRARY",
                                   path.c_str(), true);
      } else {
        SDL_UnsetEnvironmentVariable(environment, "SINEN_RENDERDOC_LIBRARY");
      }
    }
  } restore{environment, oldPath != nullptr, savedPath};

  SDL_SetEnvironmentVariable(environment, "SINEN_RENDERDOC_LIBRARY",
                             "sinen_missing_renderdoc_library", true);
  EXPECT_FALSE(RenderDoc::initialize());
  EXPECT_FALSE(RenderDoc::isAvailable());
  EXPECT_FALSE(RenderDoc::triggerCapture());
  EXPECT_FALSE(RenderDoc::isFrameCapturing());
  EXPECT_FALSE(RenderDoc::setCaptureFilePathTemplate("captures/sinen"));
  EXPECT_EQ(RenderDoc::getNumCaptures(), 0u);
  EXPECT_TRUE(RenderDoc::getCaptureFilePath(0).empty());

  SDL_SetEnvironmentVariable(environment, "SINEN_RENDERDOC_LIBRARY",
                             SINEN_TEST_RENDERDOC_PATH, true);
  EXPECT_FALSE(RenderDoc::initialize()); // Unsupported API may be retried.
  EXPECT_FALSE(RenderDoc::isAvailable());

  auto *fixture = SDL_LoadObject(SINEN_TEST_RENDERDOC_PATH);
  ASSERT_NE(fixture, nullptr) << SDL_GetError();
  const auto acceptAPI = reinterpret_cast<void (*)()>(
      SDL_LoadFunction(fixture, "fixtureAcceptAPI"));
  const auto frames = reinterpret_cast<uint32_t (*)()>(
      SDL_LoadFunction(fixture, "fixtureFrames"));
  const auto pathTemplate = reinterpret_cast<const char *(*)()>(
      SDL_LoadFunction(fixture, "fixturePathTemplate"));
  ASSERT_NE(acceptAPI, nullptr);
  ASSERT_NE(frames, nullptr);
  ASSERT_NE(pathTemplate, nullptr);
  acceptAPI();
  ASSERT_TRUE(RenderDoc::initialize());
  EXPECT_TRUE(
      RenderDoc::initialize()); // Idempotent, including backend switches.
  EXPECT_TRUE(RenderDoc::isAvailable());
  EXPECT_FALSE(RenderDoc::isFrameCapturing());
  EXPECT_EQ(RenderDoc::getNumCaptures(), 0u);
  EXPECT_FALSE(RenderDoc::triggerCapture(0));
  EXPECT_EQ(frames(), 0u);
  EXPECT_FALSE(RenderDoc::setCaptureFilePathTemplate(nullptr));
  EXPECT_FALSE(RenderDoc::setCaptureFilePathTemplate(""));
  EXPECT_TRUE(RenderDoc::setCaptureFilePathTemplate("captures/sinen"));
  EXPECT_STREQ(pathTemplate(), "captures/sinen");
  EXPECT_TRUE(RenderDoc::triggerCapture());
  EXPECT_EQ(frames(), 1u);
  EXPECT_TRUE(RenderDoc::isFrameCapturing());
  EXPECT_EQ(RenderDoc::getNumCaptures(), 1u);
  EXPECT_EQ(RenderDoc::getCaptureFilePath(0), "captures/sinen_frame42.rdc");
  EXPECT_TRUE(RenderDoc::getCaptureFilePath(1).empty());
  EXPECT_TRUE(RenderDoc::triggerCapture(3));
  EXPECT_EQ(frames(), 3u);
  SDL_UnloadObject(fixture); // The engine retains its own library reference.
  EXPECT_TRUE(RenderDoc::triggerCapture());
  EXPECT_EQ(RenderDoc::getCaptureFilePath(0), "captures/sinen_frame42.rdc");
}
