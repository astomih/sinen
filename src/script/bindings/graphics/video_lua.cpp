#include <graphics/video/video.hpp>

#include <core/buffer/buffer.hpp>
#include <script/binding.hpp>
#include <script/luaapi.hpp>

#include <cstring>

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

static int lVideoWriterNew(lua_State *L) {
  udPushPtr<VideoWriter>(L, VideoWriter::create());
  return 1;
}

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
  lua_pushboolean(L, writer->addFrame(static_cast<const UInt8 *>(buffer.data()),
                                      width, height,
                                      pixelFormatFromString(formatName), stride,
                                      quality));
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

static int lVideoWriterClose(lua_State *L) {
  auto &writer = udPtr<VideoWriter>(L, 1);
  lua_pushboolean(L, writer->close());
  return 1;
}

static int lVideoWriterFrameCount(lua_State *L) {
  auto &writer = udPtr<VideoWriter>(L, 1);
  lua_pushinteger(L, writer->frameCount());
  return 1;
}

static int lVideoWriterLastError(lua_State *L) {
  auto &writer = udPtr<VideoWriter>(L, 1);
  auto error = writer->lastError();
  lua_pushlstring(L, error.data(), error.size());
  return 1;
}

static int lVideoReaderNew(lua_State *L) {
  udPushPtr<VideoReader>(L, VideoReader::create());
  return 1;
}

static int lVideoReaderOpen(lua_State *L) {
  auto &reader = udPtr<VideoReader>(L, 1);
  const char *path = luaL_checkstring(L, 2);
  lua_pushboolean(L, reader->open(path));
  return 1;
}

static int lVideoReaderClose(lua_State *L) {
  auto &reader = udPtr<VideoReader>(L, 1);
  reader->close();
  return 0;
}

static int lVideoReaderInfo(lua_State *L) {
  auto &reader = udPtr<VideoReader>(L, 1);
  const auto &info = reader->videoInfo();
  lua_newtable(L);
  lua_pushinteger(L, info.width);
  lua_setfield(L, -2, "width");
  lua_pushinteger(L, info.height);
  lua_setfield(L, -2, "height");
  lua_pushinteger(L, info.fps);
  lua_setfield(L, -2, "fps");
  lua_pushinteger(L, info.frameCount);
  lua_setfield(L, -2, "frameCount");
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
  lua_pushinteger(L, frame.width);
  lua_setfield(L, -2, "width");
  lua_pushinteger(L, frame.height);
  lua_setfield(L, -2, "height");
  lua_pushinteger(L, frame.channels);
  lua_setfield(L, -2, "channels");
  udNewOwned<Buffer>(L, std::move(buffer));
  lua_setfield(L, -2, "pixels");
  return 1;
}

static int lVideoReaderLastError(lua_State *L) {
  auto &reader = udPtr<VideoReader>(L, 1);
  auto error = reader->lastError();
  lua_pushlstring(L, error.data(), error.size());
  return 1;
}

void registerVideo(lua_State *L) {
  Binding writer(L);
  writer.beginClass(VideoWriter::metaTableName(), udPtrGc<VideoWriter>,
                    "VideoWriter", lVideoWriterNew);
  writer.registerFunction("open", lVideoWriterOpen);
  writer.registerFunction("addFrame", lVideoWriterAddFrame);
  writer.registerFunction("addEncodedFrame", lVideoWriterAddEncodedFrame);
  writer.registerFunction("close", lVideoWriterClose);
  writer.registerFunction("frameCount", lVideoWriterFrameCount);
  writer.registerFunction("lastError", lVideoWriterLastError);
  writer.endClass();

  Binding reader(L);
  reader.beginClass(VideoReader::metaTableName(), udPtrGc<VideoReader>,
                    "VideoReader", lVideoReaderNew);
  reader.registerFunction("open", lVideoReaderOpen);
  reader.registerFunction("close", lVideoReaderClose);
  reader.registerFunction("info", lVideoReaderInfo);
  reader.registerFunction("readEncodedFrame", lVideoReaderReadEncodedFrame);
  reader.registerFunction("readFrame", lVideoReaderReadFrame);
  reader.registerFunction("lastError", lVideoReaderLastError);
  reader.endClass();
}
} // namespace sinen
