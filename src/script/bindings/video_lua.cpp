#include "luaapi.hpp"

#include <graphics/video/video.hpp>

#include <core/buffer/buffer.hpp>

#include <cstring>
#include <limits>

namespace sinen {
namespace {
VideoPixelFormat pixelFormatFromString(StringView value) {
  if (value == "rgb" || value == "RGB8" || value == "rgb8") {
    return VideoPixelFormat::RGB8;
  }
  if (value == "bgr" || value == "BGR8" || value == "bgr8") {
    return VideoPixelFormat::BGR8;
  }
  if (value == "bgra" || value == "BGRA8" || value == "bgra8") {
    return VideoPixelFormat::BGRA8;
  }
  return VideoPixelFormat::RGBA8;
}
} // namespace

static auto lVideoWriterNew() { return VideoWriter::create(); }

static int lVideoWriterOpen(lua_State *L) {
  auto &writer = udPtr<VideoWriter>(L, 1);
  const char *path = luaL_checkstring(L, 2);
  const auto width = static_cast<UInt32>(luaL_checkinteger(L, 3));
  const auto height = static_cast<UInt32>(luaL_checkinteger(L, 4));
  const auto fps = static_cast<UInt32>(luaL_checkinteger(L, 5));
  lua_pushboolean(L, writer->open(path, width, height, fps));
  return 1;
}

static int lVideoWriterAddFrame(lua_State *L) {
  auto &writer = udPtr<VideoWriter>(L, 1);
  auto &buffer = udValue<Buffer>(L, 2);
  const auto width = static_cast<UInt32>(luaL_checkinteger(L, 3));
  const auto height = static_cast<UInt32>(luaL_checkinteger(L, 4));
  const char *formatName = luaL_optstring(L, 5, "rgba8");
  const int quality = static_cast<int>(luaL_optinteger(L, 6, 90));
  const auto stride = static_cast<UInt32>(luaL_optinteger(L, 7, 0));
  const auto format = pixelFormatFromString(formatName);
  const UInt64 rowBytes =
      static_cast<UInt64>(width) *
      ((format == VideoPixelFormat::RGB8 || format == VideoPixelFormat::BGR8)
           ? 3
           : 4);
  const UInt64 actualStride = stride == 0 ? rowBytes : stride;
  const UInt64 required =
      height == 0 ? 0
                  : static_cast<UInt64>(height - 1) * actualStride + rowBytes;
  if (rowBytes > std::numeric_limits<UInt32>::max() ||
      actualStride < rowBytes || required > buffer.size()) {
    luaL_error(L, "video frame buffer is too small or stride is invalid");
  }
  lua_pushboolean(L, writer->addFrame(static_cast<const UInt8 *>(buffer.data()),
                                      width, height, format, stride, quality));
  return 1;
}

static int lVideoWriterAddEncodedFrame(lua_State *L) {
  auto &writer = udPtr<VideoWriter>(L, 1);
  auto &buffer = udValue<Buffer>(L, 2);
  lua_pushboolean(
      L, writer->addEncodedFrame(static_cast<const UInt8 *>(buffer.data()),
                                 static_cast<Size>(buffer.size())));
  return 1;
}

static auto lVideoWriterClose(Ptr<VideoWriter> &writer) {
  return writer->close();
}

static auto lVideoWriterFrameCount(Ptr<VideoWriter> &writer) {
  return writer->frameCount();
}

static auto lVideoWriterLastError(Ptr<VideoWriter> &writer) {
  auto error = writer->lastError();
  return error;
}

static auto lVideoReaderNew() { return VideoReader::create(); }

static auto lVideoReaderOpen(Ptr<VideoReader> &reader, const char *path) {
  return reader->open(path);
}

static auto lVideoReaderClose(Ptr<VideoReader> &reader) {
  reader->close();
  return;
}

static int lVideoReaderInfo(lua_State *L) {
  auto &reader = udPtr<VideoReader>(L, 1);
  const auto &info = reader->videoInfo();
  lua_newtable(L);
  luau::table(L, -1).value("width", info.width);
  luau::table(L, -1).value("height", info.height);
  luau::table(L, -1).value("fps", info.fps);
  luau::table(L, -1).value("frameCount", info.frameCount);
  return 1;
}

static int lVideoReaderReadEncodedFrame(lua_State *L) {
  auto &reader = udPtr<VideoReader>(L, 1);
  const auto index = static_cast<UInt32>(luaL_checkinteger(L, 2));
  Array<UInt8> encoded;
  if (!reader->readEncodedFrame(index, encoded)) {
    lua_pushnil(L);
    return 1;
  }
  auto buffer = makeBuffer(encoded.size(), BufferType::Binary);
  std::memcpy(buffer.data(), encoded.data(), encoded.size());
  udNewOwned<Buffer>(L, std::move(buffer));
  return 1;
}

static int lVideoReaderReadFrame(lua_State *L) {
  auto &reader = udPtr<VideoReader>(L, 1);
  const auto index = static_cast<UInt32>(luaL_checkinteger(L, 2));
  VideoFrame frame;
  if (!reader->readFrame(index, frame)) {
    lua_pushnil(L);
    return 1;
  }
  auto buffer = makeBuffer(frame.pixels.size(), BufferType::Binary);
  std::memcpy(buffer.data(), frame.pixels.data(), frame.pixels.size());
  lua_newtable(L);
  luau::table(L, -1).value("width", frame.width);
  luau::table(L, -1).value("height", frame.height);
  luau::table(L, -1).value("channels", frame.channels);
  udNewOwned<Buffer>(L, std::move(buffer));
  lua_setfield(L, -2, "pixels");
  return 1;
}

static auto lVideoReaderLastError(Ptr<VideoReader> &reader) {
  auto error = reader->lastError();
  return error;
}

void registerVideo(luau::table &sn) {
  auto videoWriter =
      engineClass<Ptr<VideoWriter>>(sn, VideoWriter::metaTableName());

  videoWriter.static_function<lVideoWriterNew>("new");

  videoWriter.raw_method<lVideoWriterOpen>("open");
  videoWriter.raw_method<lVideoWriterAddFrame>("addFrame");
  videoWriter.raw_method<lVideoWriterAddEncodedFrame>("addEncodedFrame");
  videoWriter.method<lVideoWriterClose>("close");
  videoWriter.method<lVideoWriterFrameCount>("frameCount");
  videoWriter.method<lVideoWriterLastError>("lastError");

  auto videoReader =
      engineClass<Ptr<VideoReader>>(sn, VideoReader::metaTableName());

  videoReader.static_function<lVideoReaderNew>("new");

  videoReader.method<lVideoReaderOpen>("open");
  videoReader.method<lVideoReaderClose>("close");
  videoReader.raw_method<lVideoReaderInfo>("info");
  videoReader.raw_method<lVideoReaderReadEncodedFrame>("readEncodedFrame");
  videoReader.raw_method<lVideoReaderReadFrame>("readFrame");
  videoReader.method<lVideoReaderLastError>("lastError");
}
} // namespace sinen
