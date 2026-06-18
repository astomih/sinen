#ifndef SINEN_VIDEO_HPP
#define SINEN_VIDEO_HPP

#include <core/data/array.hpp>
#include <core/data/ptr.hpp>
#include <core/data/string.hpp>
#include <core/def/types.hpp>

#include <fstream>

namespace sinen {
enum class VideoPixelFormat {
  RGB8,
  RGBA8,
  BGR8,
  BGRA8,
};

struct VideoInfo {
  UInt32 width = 0;
  UInt32 height = 0;
  UInt32 fps = 0;
  UInt32 frameCount = 0;
};

struct VideoFrame {
  UInt32 width = 0;
  UInt32 height = 0;
  UInt32 channels = 4;
  Array<UInt8> pixels;
};

class VideoWriter {
public:
  static Ptr<VideoWriter> create();
  static constexpr const char *metaTableName() { return "sn.VideoWriter"; }

  VideoWriter() = default;
  ~VideoWriter();

  bool open(StringView path, UInt32 width, UInt32 height, UInt32 fps);
  bool addFrame(const UInt8 *pixels, UInt32 width, UInt32 height,
                VideoPixelFormat format = VideoPixelFormat::RGBA8,
                UInt32 strideBytes = 0, int quality = 90);
  bool addEncodedFrame(const UInt8 *encodedData, Size encodedSize);
  bool close();

  bool isOpen() const;
  UInt32 frameCount() const;
  StringView lastError() const;

private:
  struct IndexEntry {
    UInt32 offset = 0;
    UInt32 size = 0;
  };

  void setError(StringView message);
  bool writeHeaders();
  bool patchHeaders();

  std::ofstream output;
  String error;
  VideoInfo info;
  Array<IndexEntry> index;
  UInt32 suggestedBufferSize = 0;
  std::streampos riffSizePos = 0;
  std::streampos hdrlSizePos = 0;
  std::streampos avihFramesPos = 0;
  std::streampos avihSuggestedBufferSizePos = 0;
  std::streampos strhLengthPos = 0;
  std::streampos strhSuggestedBufferSizePos = 0;
  std::streampos moviSizePos = 0;
  std::streampos moviDataStart = 0;
};

class VideoReader {
public:
  static Ptr<VideoReader> create();
  static constexpr const char *metaTableName() { return "sn.VideoReader"; }

  bool open(StringView path);
  void close();

  const VideoInfo &videoInfo() const;
  bool readEncodedFrame(UInt32 index, Array<UInt8> &out) const;
  bool readFrame(UInt32 index, VideoFrame &out) const;
  StringView lastError() const;

private:
  struct FrameEntry {
    Size offset = 0;
    Size size = 0;
  };

  void setError(StringView message);
  bool parse();
  bool parseHdrl(Size begin, Size end);
  bool parseIdx1(Size begin, Size size);
  bool scanMovi();

  Array<UInt8> bytes;
  Array<FrameEntry> frames;
  VideoInfo info;
  String error;
  Size moviDataStart = 0;
  Size moviListTypePos = 0;
  Size moviEnd = 0;
};
} // namespace sinen

#endif // SINEN_VIDEO_HPP
