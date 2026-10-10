#include <graphics/video/video.hpp>
#include <gtest/gtest.h>
#include <minimp4.h>
#include <platform/io/filesystem.hpp>
#include <script/script.hpp>
#include <wels/codec_api.h>

#include <cstring>
#include <filesystem>
#include <fstream>
#include <vector>

// Supply image decoding without pulling in the texture/GPU runtime, and a
// script path without initializing the scripting runtime for these file tests.
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
namespace sinen {
String Script::getBaseDirectory() { return "."; }
} // namespace sinen

namespace {
class VideoTests : public testing::Test {
protected:
  sinen::String path = "user://sinen_video_encoder_test.mp4";
  sinen::String resolved;
  void SetUp() override {
    ASSERT_TRUE(sinen::Filesystem::resolveSandboxPath(
        path, sinen::FilesystemAccess::Write, resolved));
  }
  void TearDown() override {
    std::error_code error;
    std::filesystem::remove(resolved.c_str(), error);
  }
};

struct Demux {
  std::vector<unsigned char> bytes;
  MP4D_demux_t mp4{};
  ~Demux() { MP4D_close(&mp4); }
  bool open(const char *path) {
    std::ifstream input(path, std::ios::binary);
    bytes.assign(std::istreambuf_iterator<char>(input), {});
    return MP4D_open(&mp4, read, this, bytes.size()) != 0;
  }
  static int read(int64_t offset, void *buffer, size_t size, void *token) {
    const auto &bytes = static_cast<Demux *>(token)->bytes;
    if (offset < 0 || static_cast<uint64_t>(offset) > bytes.size() ||
        size > bytes.size() - static_cast<size_t>(offset)) {
      return 1;
    }
    std::memcpy(buffer, bytes.data() + offset, size);
    return 0;
  }
};

struct Decoder {
  ISVCDecoder *decoder = nullptr;
  Decoder() { WelsCreateDecoder(&decoder); }
  ~Decoder() {
    if (decoder) {
      decoder->Uninitialize();
      WelsDestroyDecoder(decoder);
    }
  }
};

void appendNal(std::vector<unsigned char> &out, const void *data, size_t size) {
  out.insert(out.end(), {0, 0, 0, 1});
  const auto *bytes = static_cast<const unsigned char *>(data);
  out.insert(out.end(), bytes, bytes + size);
}
} // namespace

TEST_F(VideoTests, WritesDecodableH264WithExactTimingAndAllPixelFormats) {
  // Non-macroblock dimensions exercise SPS cropping. Padding exercises stride.
  constexpr unsigned width = 34, height = 18, fps = 29, frameCount = 65;
  sinen::VideoWriter writer;
  ASSERT_TRUE(writer.open(path, width, height, fps)) << writer.lastError();
  const sinen::VideoPixelFormat formats[] = {
      sinen::VideoPixelFormat::RGB8, sinen::VideoPixelFormat::RGBA8,
      sinen::VideoPixelFormat::BGR8, sinen::VideoPixelFormat::BGRA8};
  for (unsigned i = 0; i < frameCount; ++i) {
    const auto format = formats[i % 4];
    const unsigned channels = i % 2 == 0 ? 3 : 4;
    const unsigned stride = width * channels + 7;
    std::vector<unsigned char> pixels(stride * height, 0);
    for (unsigned y = 0; y < height; ++y) {
      for (unsigned x = 0; x < width; ++x) {
        auto *p = pixels.data() + y * stride + x * channels;
        p[i % 4 >= 2 ? 2 : 0] = 255; // red in all four source formats
      }
    }
    ASSERT_TRUE(writer.addFrame(pixels.data(), width, height, format, stride,
                                i % 2 ? 90 : 100))
        << writer.lastError();
  }
  EXPECT_EQ(writer.frameCount(), frameCount);
  ASSERT_TRUE(writer.close()) << writer.lastError();
  EXPECT_TRUE(writer.close());
  EXPECT_FALSE(writer.isOpen());

  Demux file;
  ASSERT_TRUE(file.open(resolved.c_str()));
  ASSERT_EQ(file.mp4.track_count, 1u);
  const auto &track = file.mp4.track[0];
  EXPECT_EQ(track.object_type_indication, MP4_OBJECT_TYPE_AVC);
  EXPECT_EQ(track.SampleDescription.video.width, width);
  EXPECT_EQ(track.SampleDescription.video.height, height);
  EXPECT_EQ(track.timescale, fps);
  EXPECT_EQ(track.sample_count, frameCount);
  EXPECT_EQ(track.duration_lo, frameCount);

  Decoder decoder;
  ASSERT_NE(decoder.decoder, nullptr);
  SDecodingParam params{};
  params.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_AVC;
  ASSERT_EQ(decoder.decoder->Initialize(&params), 0);
  std::vector<unsigned char> annexB;
  int size = 0;
  const void *sps = MP4D_read_sps(&file.mp4, 0, 0, &size);
  ASSERT_NE(sps, nullptr);
  appendNal(annexB, sps, size);
  const void *pps = MP4D_read_pps(&file.mp4, 0, 0, &size);
  ASSERT_NE(pps, nullptr);
  appendNal(annexB, pps, size);
  for (unsigned i = 0; i < frameCount; ++i) {
    unsigned sampleSize = 0, timestamp = 0, duration = 0;
    const auto offset =
        MP4D_frame_offset(&file.mp4, 0, i, &sampleSize, &timestamp, &duration);
    EXPECT_EQ(timestamp, i);
    EXPECT_EQ(duration, 1u);
    ASSERT_LE(offset + sampleSize, file.bytes.size());
    const auto *sample = file.bytes.data() + offset;
    size_t pos = 0;
    while (pos < sampleSize) {
      ASSERT_LE(pos + 4, sampleSize);
      const size_t nalSize = (size_t(sample[pos]) << 24) |
                             (size_t(sample[pos + 1]) << 16) |
                             (size_t(sample[pos + 2]) << 8) | sample[pos + 3];
      pos += 4;
      ASSERT_LE(nalSize, sampleSize - pos);
      appendNal(annexB, sample + pos, nalSize);
      pos += nalSize;
    }
    unsigned char *planes[3]{};
    SBufferInfo decoded{};
    ASSERT_EQ(
        decoder.decoder->DecodeFrameNoDelay(
            annexB.data(), static_cast<int>(annexB.size()), planes, &decoded),
        dsErrorFree);
    ASSERT_EQ(decoded.iBufferStatus, 1);
    EXPECT_EQ(decoded.UsrData.sSystemBuffer.iWidth, width);
    EXPECT_EQ(decoded.UsrData.sSystemBuffer.iHeight, height);
    // BT.601 red is approximately Y=82, U=90, V=240.
    EXPECT_NEAR(planes[0][0], 82, 5);
    EXPECT_NEAR(planes[1][0], 90, 5);
    EXPECT_NEAR(planes[2][0], 240, 5);
    annexB.clear();
  }
}

TEST_F(VideoTests, RejectsInvalidInputAndCanReopen) {
  sinen::VideoWriter writer;
  EXPECT_FALSE(writer.addFrame(nullptr, 32, 32));
  EXPECT_FALSE(writer.open(path, 31, 32, 30));
  EXPECT_FALSE(writer.isOpen());
  EXPECT_FALSE(writer.open(path, 32, 32, 0));
  EXPECT_FALSE(writer.open(path, 32, 32, 61));
  ASSERT_TRUE(writer.open(path, 32, 32, 30));
  std::vector<unsigned char> pixels(32 * 32 * 4, 255);
  EXPECT_FALSE(writer.addFrame(nullptr, 32, 32));
  EXPECT_FALSE(writer.addFrame(pixels.data(), 30, 32));
  EXPECT_FALSE(writer.addFrame(pixels.data(), 32, 32,
                               sinen::VideoPixelFormat::RGBA8, 1));
  EXPECT_FALSE(writer.addEncodedFrame(pixels.data(), pixels.size()));
  EXPECT_EQ(writer.frameCount(), 0u);
  ASSERT_TRUE(writer.addFrame(pixels.data(), 32, 32));
  ASSERT_TRUE(writer.close());
  ASSERT_TRUE(writer.open(path, 32, 32, 30));
  EXPECT_EQ(writer.frameCount(), 0u);
  ASSERT_TRUE(writer.addFrame(pixels.data(), 32, 32));
  ASSERT_TRUE(writer.close());
}

TEST_F(VideoTests, DestructorFinalizesFileAndExtensionIsCaseInsensitive) {
  path = "user://sinen_video_encoder_test.MP4";
  ASSERT_TRUE(sinen::Filesystem::resolveSandboxPath(
      path, sinen::FilesystemAccess::Write, resolved));
  {
    sinen::VideoWriter writer;
    ASSERT_TRUE(writer.open(path, 32, 32, 24));
    std::vector<unsigned char> pixels(32 * 32 * 4, 128);
    ASSERT_TRUE(writer.addFrame(pixels.data(), 32, 32));
  }
  Demux file;
  ASSERT_TRUE(file.open(resolved.c_str()));
  ASSERT_EQ(file.mp4.track_count, 1u);
  EXPECT_EQ(file.mp4.track[0].sample_count, 1u);
}

TEST_F(VideoTests, PreservesAviMjpegWriterAndReader) {
  path = "user://sinen_video_encoder_test.avi";
  ASSERT_TRUE(sinen::Filesystem::resolveSandboxPath(
      path, sinen::FilesystemAccess::Write, resolved));
  sinen::VideoWriter writer;
  ASSERT_TRUE(writer.open(path, 32, 32, 24));
  std::vector<unsigned char> pixels(32 * 32 * 4, 128);
  ASSERT_TRUE(writer.addFrame(pixels.data(), 32, 32));
  ASSERT_TRUE(writer.close());
  sinen::VideoReader reader;
  ASSERT_TRUE(reader.open(path));
  EXPECT_EQ(reader.videoInfo().frameCount, 1u);
  sinen::VideoFrame decoded;
  ASSERT_TRUE(reader.readFrame(0, decoded));
  EXPECT_EQ(decoded.width, 32u);
  EXPECT_EQ(decoded.height, 32u);
}
