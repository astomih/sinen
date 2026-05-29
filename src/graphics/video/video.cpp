#include <graphics/video/video.hpp>
#include <platform/io/asset_reader.hpp>
#include <platform/io/filesystem.hpp>

#include <algorithm>
#include <cstring>
#include <limits>
#include <memory>
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace sinen {
namespace {
constexpr UInt32 AviHasIndex = 0x00000010;

UInt32 fourcc(char a, char b, char c, char d) {
  return static_cast<UInt32>(static_cast<UInt8>(a)) |
         (static_cast<UInt32>(static_cast<UInt8>(b)) << 8) |
         (static_cast<UInt32>(static_cast<UInt8>(c)) << 16) |
         (static_cast<UInt32>(static_cast<UInt8>(d)) << 24);
}

void writeU16(std::ostream &os, UInt32 value) {
  char bytes[2] = {static_cast<char>(value & 0xff),
                   static_cast<char>((value >> 8) & 0xff)};
  os.write(bytes, sizeof(bytes));
}

void writeU32(std::ostream &os, UInt32 value) {
  char bytes[4] = {static_cast<char>(value & 0xff),
                   static_cast<char>((value >> 8) & 0xff),
                   static_cast<char>((value >> 16) & 0xff),
                   static_cast<char>((value >> 24) & 0xff)};
  os.write(bytes, sizeof(bytes));
}

void writeFourcc(std::ostream &os, UInt32 value) { writeU32(os, value); }

bool patchU32(std::ofstream &stream, std::streampos pos, UInt32 value) {
  auto current = stream.tellp();
  stream.seekp(pos);
  if (!stream) {
    return false;
  }
  writeU32(stream, value);
  stream.seekp(current);
  return static_cast<bool>(stream);
}

bool fitsU32(UInt64 value) {
  return value <= std::numeric_limits<UInt32>::max();
}

UInt32 clampU32(UInt64 value) {
  return fitsU32(value) ? static_cast<UInt32>(value)
                        : std::numeric_limits<UInt32>::max();
}

Size padded(Size size) { return size + (size & 1U); }

UInt32 readU16(const Array<UInt8> &bytes, Size pos) {
  return static_cast<UInt32>(bytes[pos]) |
         (static_cast<UInt32>(bytes[pos + 1]) << 8);
}

UInt32 readU32(const Array<UInt8> &bytes, Size pos) {
  return static_cast<UInt32>(bytes[pos]) |
         (static_cast<UInt32>(bytes[pos + 1]) << 8) |
         (static_cast<UInt32>(bytes[pos + 2]) << 16) |
         (static_cast<UInt32>(bytes[pos + 3]) << 24);
}

bool hasRange(Size size, Size offset, Size length) {
  return offset <= size && length <= size - offset;
}

void jpgWriteCallback(void *context, void *data, int size) {
  auto *out = static_cast<Array<UInt8> *>(context);
  const auto *begin = static_cast<const UInt8 *>(data);
  out->insert(out->end(), begin, begin + size);
}

Array<UInt8> toRgb(const UInt8 *pixels, UInt32 width, UInt32 height,
                   VideoPixelFormat format, UInt32 strideBytes) {
  const UInt32 sourceChannels =
      (format == VideoPixelFormat::RGB8 || format == VideoPixelFormat::BGR8)
          ? 3
          : 4;
  if (strideBytes == 0) {
    strideBytes = width * sourceChannels;
  }

  Array<UInt8> rgb;
  rgb.resize(static_cast<Size>(width) * height * 3);
  for (UInt32 y = 0; y < height; ++y) {
    const UInt8 *src = pixels + static_cast<Size>(y) * strideBytes;
    UInt8 *dst = rgb.data() + static_cast<Size>(y) * width * 3;
    for (UInt32 x = 0; x < width; ++x) {
      const UInt8 *p = src + static_cast<Size>(x) * sourceChannels;
      switch (format) {
      case VideoPixelFormat::RGB8:
      case VideoPixelFormat::RGBA8:
        dst[x * 3 + 0] = p[0];
        dst[x * 3 + 1] = p[1];
        dst[x * 3 + 2] = p[2];
        break;
      case VideoPixelFormat::BGR8:
      case VideoPixelFormat::BGRA8:
        dst[x * 3 + 0] = p[2];
        dst[x * 3 + 1] = p[1];
        dst[x * 3 + 2] = p[0];
        break;
      }
    }
  }
  return rgb;
}

bool chunkAt(const Array<UInt8> &bytes, Size pos, UInt32 expectedSize) {
  if (!hasRange(bytes.size(), pos, 8)) {
    return false;
  }
  const UInt32 id = readU32(bytes, pos);
  if (id != fourcc('0', '0', 'd', 'c') && id != fourcc('0', '0', 'd', 'b')) {
    return false;
  }
  const UInt32 size = readU32(bytes, pos + 4);
  return size == expectedSize && hasRange(bytes.size(), pos + 8, size);
}

} // namespace

Ptr<VideoWriter> VideoWriter::create() {
  return std::make_shared<VideoWriter>();
}

VideoWriter::~VideoWriter() { close(); }

void VideoWriter::setError(StringView message) { error = message; }

bool VideoWriter::open(StringView path, UInt32 width, UInt32 height,
                       UInt32 fps) {
  close();
  error.clear();
  if (width == 0 || height == 0 || fps == 0) {
    setError("invalid video dimensions or FPS");
    return false;
  }

  String resolvedPath;
  if (!Filesystem::resolveSandboxPath(path, FilesystemAccess::Write,
                                      resolvedPath)) {
    setError("video output path is outside the filesystem sandbox");
    return false;
  }

  output.open(resolvedPath.c_str(), std::ios::binary | std::ios::trunc);
  if (!output) {
    setError("failed to open video output file");
    return false;
  }

  info = {width, height, fps, 0};
  index.clear();
  suggestedBufferSize = 0;
  if (!writeHeaders()) {
    close();
    return false;
  }
  return true;
}

bool VideoWriter::writeHeaders() {
  writeFourcc(output, fourcc('R', 'I', 'F', 'F'));
  riffSizePos = output.tellp();
  writeU32(output, 0);
  writeFourcc(output, fourcc('A', 'V', 'I', ' '));

  writeFourcc(output, fourcc('L', 'I', 'S', 'T'));
  hdrlSizePos = output.tellp();
  writeU32(output, 0);
  writeFourcc(output, fourcc('h', 'd', 'r', 'l'));

  writeFourcc(output, fourcc('a', 'v', 'i', 'h'));
  writeU32(output, 56);
  writeU32(output, 1000000U / info.fps);
  writeU32(output, clampU32(static_cast<UInt64>(info.width) * info.height * 3 *
                            info.fps));
  writeU32(output, 0);
  writeU32(output, AviHasIndex);
  avihFramesPos = output.tellp();
  writeU32(output, 0);
  writeU32(output, 0);
  writeU32(output, 1);
  avihSuggestedBufferSizePos = output.tellp();
  writeU32(output, 0);
  writeU32(output, info.width);
  writeU32(output, info.height);
  writeU32(output, 0);
  writeU32(output, 0);
  writeU32(output, 0);
  writeU32(output, 0);

  writeFourcc(output, fourcc('L', 'I', 'S', 'T'));
  const auto strlSizePos = output.tellp();
  writeU32(output, 0);
  const auto strlStart = output.tellp();
  writeFourcc(output, fourcc('s', 't', 'r', 'l'));

  writeFourcc(output, fourcc('s', 't', 'r', 'h'));
  writeU32(output, 56);
  writeFourcc(output, fourcc('v', 'i', 'd', 's'));
  writeFourcc(output, fourcc('M', 'J', 'P', 'G'));
  writeU32(output, 0);
  writeU16(output, 0);
  writeU16(output, 0);
  writeU32(output, 0);
  writeU32(output, 1);
  writeU32(output, info.fps);
  writeU32(output, 0);
  strhLengthPos = output.tellp();
  writeU32(output, 0);
  strhSuggestedBufferSizePos = output.tellp();
  writeU32(output, 0);
  writeU32(output, 0xffffffffU);
  writeU32(output, 0);
  writeU16(output, 0);
  writeU16(output, 0);
  writeU16(output, static_cast<UInt32>(info.width));
  writeU16(output, static_cast<UInt32>(info.height));

  writeFourcc(output, fourcc('s', 't', 'r', 'f'));
  writeU32(output, 40);
  writeU32(output, 40);
  writeU32(output, info.width);
  writeU32(output, info.height);
  writeU16(output, 1);
  writeU16(output, 24);
  writeFourcc(output, fourcc('M', 'J', 'P', 'G'));
  writeU32(output, clampU32(static_cast<UInt64>(info.width) * info.height * 3));
  writeU32(output, 0);
  writeU32(output, 0);
  writeU32(output, 0);
  writeU32(output, 0);

  const auto strlEnd = output.tellp();
  patchU32(output, strlSizePos, static_cast<UInt32>(strlEnd - strlStart));

  const auto hdrlEnd = output.tellp();
  patchU32(output, hdrlSizePos,
           static_cast<UInt32>(hdrlEnd - (hdrlSizePos + std::streamoff(4))));

  writeFourcc(output, fourcc('L', 'I', 'S', 'T'));
  moviSizePos = output.tellp();
  writeU32(output, 0);
  writeFourcc(output, fourcc('m', 'o', 'v', 'i'));
  moviDataStart = output.tellp();

  if (!output) {
    setError("failed to write video headers");
    return false;
  }
  return true;
}

bool VideoWriter::addFrame(const UInt8 *pixels, UInt32 width, UInt32 height,
                           VideoPixelFormat format, UInt32 strideBytes,
                           int quality) {
  if (!isOpen()) {
    setError("video writer is not open");
    return false;
  }
  if (!pixels || width != info.width || height != info.height) {
    setError("video frame dimensions do not match the stream");
    return false;
  }
  quality = std::clamp(quality, 1, 100);

  Array<UInt8> rgb = toRgb(pixels, width, height, format, strideBytes);
  Array<UInt8> jpeg;
  const int ok =
      stbi_write_jpg_to_func(jpgWriteCallback, &jpeg, static_cast<int>(width),
                             static_cast<int>(height), 3, rgb.data(), quality);
  if (!ok || jpeg.empty()) {
    setError("failed to encode video frame");
    return false;
  }
  return addEncodedFrame(jpeg.data(), jpeg.size());
}

bool VideoWriter::addEncodedFrame(const UInt8 *encodedData, Size encodedSize) {
  if (!isOpen()) {
    setError("video writer is not open");
    return false;
  }
  if (!encodedData || encodedSize == 0 || !fitsU32(encodedSize)) {
    setError("invalid encoded video frame data");
    return false;
  }

  const auto chunkStart = output.tellp();
  const UInt64 offset64 =
      static_cast<UInt64>(chunkStart - (moviDataStart - std::streamoff(4)));
  if (!fitsU32(offset64)) {
    setError("video files over 4GB are not supported");
    return false;
  }

  writeFourcc(output, fourcc('0', '0', 'd', 'c'));
  writeU32(output, static_cast<UInt32>(encodedSize));
  output.write(reinterpret_cast<const char *>(encodedData),
               static_cast<std::streamsize>(encodedSize));
  if (encodedSize & 1U) {
    output.put('\0');
  }
  if (!output) {
    setError("failed to write encoded video frame");
    return false;
  }

  index.push_back(
      {static_cast<UInt32>(offset64), static_cast<UInt32>(encodedSize)});
  info.frameCount = static_cast<UInt32>(index.size());
  suggestedBufferSize =
      std::max(suggestedBufferSize, static_cast<UInt32>(encodedSize));
  return true;
}

bool VideoWriter::patchHeaders() {
  const auto fileEnd = output.tellp();
  const UInt64 riffSize =
      static_cast<UInt64>(static_cast<std::streamoff>(fileEnd)) - 8;
  const UInt64 moviSize =
      static_cast<UInt64>(fileEnd - (moviSizePos + std::streamoff(4)));
  if (!fitsU32(riffSize) || !fitsU32(moviSize)) {
    setError("video files over 4GB are not supported");
    return false;
  }

  return patchU32(output, riffSizePos, static_cast<UInt32>(riffSize)) &&
         patchU32(output, moviSizePos, static_cast<UInt32>(moviSize)) &&
         patchU32(output, avihFramesPos, info.frameCount) &&
         patchU32(output, avihSuggestedBufferSizePos, suggestedBufferSize) &&
         patchU32(output, strhLengthPos, info.frameCount) &&
         patchU32(output, strhSuggestedBufferSizePos, suggestedBufferSize);
}

bool VideoWriter::close() {
  if (!output.is_open()) {
    return true;
  }

  writeFourcc(output, fourcc('i', 'd', 'x', '1'));
  const UInt64 idxSize64 = static_cast<UInt64>(index.size()) * 16;
  if (!fitsU32(idxSize64)) {
    setError("video index is too large");
    output.close();
    return false;
  }
  writeU32(output, static_cast<UInt32>(idxSize64));
  for (const auto &entry : index) {
    writeFourcc(output, fourcc('0', '0', 'd', 'c'));
    writeU32(output, 0x10);
    writeU32(output, entry.offset);
    writeU32(output, entry.size);
  }

  const bool ok = output && patchHeaders();
  output.close();
  if (!ok && error.empty()) {
    setError("failed to finalize video file");
  }
  return ok;
}

bool VideoWriter::isOpen() const { return output.is_open(); }
UInt32 VideoWriter::frameCount() const { return info.frameCount; }
StringView VideoWriter::lastError() const { return error; }

Ptr<VideoReader> VideoReader::create() {
  return std::make_shared<VideoReader>();
}

void VideoReader::setError(StringView message) { error = message; }

bool VideoReader::open(StringView path) {
  close();
  error.clear();

  if (AssetReader::isArchiveMounted() && AssetReader::exists(path)) {
    String data = AssetReader::openAsString(path);
    bytes.assign(data.begin(), data.end());
    if (bytes.empty()) {
      setError("empty video input file");
      return false;
    }
    if (!parse()) {
      close();
      return false;
    }
    return true;
  }

  String resolvedPath;
  if (!Filesystem::resolveSandboxPath(path, FilesystemAccess::Read,
                                      resolvedPath)) {
    setError("video input path is outside the filesystem sandbox");
    return false;
  }

  std::ifstream input(resolvedPath.c_str(), std::ios::binary);
  if (!input) {
    setError("failed to open video input file");
    return false;
  }
  input.seekg(0, std::ios::end);
  const std::streamoff size = input.tellg();
  if (size <= 0) {
    setError("empty video input file");
    return false;
  }
  input.seekg(0, std::ios::beg);
  bytes.resize(static_cast<Size>(size));
  input.read(reinterpret_cast<char *>(bytes.data()), size);
  if (!input) {
    setError("failed to read video input file");
    close();
    return false;
  }

  if (!parse()) {
    close();
    return false;
  }
  return true;
}

void VideoReader::close() {
  bytes.clear();
  frames.clear();
  info = {};
  moviDataStart = 0;
  moviListTypePos = 0;
  moviEnd = 0;
}

bool VideoReader::parse() {
  if (!hasRange(bytes.size(), 0, 12) ||
      readU32(bytes, 0) != fourcc('R', 'I', 'F', 'F') ||
      readU32(bytes, 8) != fourcc('A', 'V', 'I', ' ')) {
    setError("input is not an AVI file");
    return false;
  }

  Size pos = 12;
  while (hasRange(bytes.size(), pos, 8)) {
    const UInt32 id = readU32(bytes, pos);
    const UInt32 size = readU32(bytes, pos + 4);
    const Size chunkData = pos + 8;
    if (!hasRange(bytes.size(), chunkData, size)) {
      break;
    }

    if (id == fourcc('L', 'I', 'S', 'T') && size >= 4) {
      const UInt32 listType = readU32(bytes, chunkData);
      if (listType == fourcc('h', 'd', 'r', 'l')) {
        if (!parseHdrl(chunkData + 4, chunkData + size)) {
          return false;
        }
      } else if (listType == fourcc('m', 'o', 'v', 'i')) {
        moviListTypePos = chunkData;
        moviDataStart = chunkData + 4;
        moviEnd = chunkData + size;
      }
    } else if (id == fourcc('i', 'd', 'x', '1')) {
      parseIdx1(chunkData, size);
    }

    pos = chunkData + padded(size);
  }

  if (moviDataStart == 0) {
    setError("AVI file has no movi list");
    return false;
  }
  if (frames.empty() && !scanMovi()) {
    return false;
  }
  info.frameCount = static_cast<UInt32>(frames.size());
  if (info.width == 0 || info.height == 0) {
    setError("video file has no usable stream format");
    return false;
  }
  return true;
}

bool VideoReader::parseHdrl(Size begin, Size end) {
  Size pos = begin;
  while (hasRange(bytes.size(), pos, 8) && pos + 8 <= end) {
    const UInt32 id = readU32(bytes, pos);
    const UInt32 size = readU32(bytes, pos + 4);
    const Size chunkData = pos + 8;
    if (!hasRange(bytes.size(), chunkData, size) || chunkData + size > end) {
      return false;
    }

    if (id == fourcc('a', 'v', 'i', 'h') && size >= 40) {
      const UInt32 usec = readU32(bytes, chunkData);
      info.width = readU32(bytes, chunkData + 32);
      info.height = readU32(bytes, chunkData + 36);
      if (usec != 0) {
        info.fps = std::max<UInt32>(1, 1000000U / usec);
      }
    } else if (id == fourcc('L', 'I', 'S', 'T') && size >= 4 &&
               readU32(bytes, chunkData) == fourcc('s', 't', 'r', 'l')) {
      if (!parseHdrl(chunkData + 4, chunkData + size)) {
        return false;
      }
    } else if (id == fourcc('s', 't', 'r', 'h') && size >= 32) {
      const UInt32 scale = readU32(bytes, chunkData + 20);
      const UInt32 rate = readU32(bytes, chunkData + 24);
      if (scale != 0 && rate != 0) {
        info.fps = std::max<UInt32>(1, rate / scale);
      }
    } else if (id == fourcc('s', 't', 'r', 'f') && size >= 20) {
      info.width = readU32(bytes, chunkData + 4);
      info.height = readU32(bytes, chunkData + 8);
      const UInt32 planes = readU16(bytes, chunkData + 12);
      const UInt32 bitCount = readU16(bytes, chunkData + 14);
      const UInt32 compression = readU32(bytes, chunkData + 16);
      if (planes != 1 || bitCount != 24 ||
          compression != fourcc('M', 'J', 'P', 'G')) {
        setError("video stream uses an unsupported encoded format");
        return false;
      }
    }

    pos = chunkData + padded(size);
  }
  return true;
}

bool VideoReader::parseIdx1(Size begin, Size size) {
  if (moviDataStart == 0 || size < 16) {
    return false;
  }

  Array<FrameEntry> parsed;
  const Size count = size / 16;
  parsed.reserve(count);
  for (Size i = 0; i < count; ++i) {
    const Size entry = begin + i * 16;
    const UInt32 id = readU32(bytes, entry);
    if (id != fourcc('0', '0', 'd', 'c') && id != fourcc('0', '0', 'd', 'b')) {
      continue;
    }
    const UInt32 offset = readU32(bytes, entry + 8);
    const UInt32 frameSize = readU32(bytes, entry + 12);
    const Size candidates[] = {
        moviDataStart + offset,
        moviListTypePos + offset,
        moviDataStart + offset - std::min<Size>(offset, 4),
        moviListTypePos + offset + 4,
    };

    bool found = false;
    for (Size chunkPos : candidates) {
      if (chunkAt(bytes, chunkPos, frameSize)) {
        parsed.push_back({chunkPos + 8, frameSize});
        found = true;
        break;
      }
    }
    if (!found && hasRange(bytes.size(), moviDataStart + offset, frameSize)) {
      parsed.push_back({moviDataStart + offset, frameSize});
    }
  }
  if (!parsed.empty()) {
    frames = std::move(parsed);
  }
  return !frames.empty();
}

bool VideoReader::scanMovi() {
  Size pos = moviDataStart;
  while (hasRange(bytes.size(), pos, 8) && pos + 8 <= moviEnd) {
    const UInt32 id = readU32(bytes, pos);
    const UInt32 size = readU32(bytes, pos + 4);
    const Size data = pos + 8;
    if (!hasRange(bytes.size(), data, size) || data + size > moviEnd) {
      break;
    }
    if (id == fourcc('0', '0', 'd', 'c') || id == fourcc('0', '0', 'd', 'b')) {
      frames.push_back({data, size});
    }
    pos = data + padded(size);
  }

  if (frames.empty()) {
    setError("video file has no encoded frames");
    return false;
  }
  return true;
}

const VideoInfo &VideoReader::videoInfo() const { return info; }

bool VideoReader::readEncodedFrame(UInt32 index, Array<UInt8> &out) const {
  if (index >= frames.size()) {
    return false;
  }
  const auto &frame = frames[index];
  out.assign(bytes.begin() + frame.offset,
             bytes.begin() + frame.offset + frame.size);
  return true;
}

bool VideoReader::readFrame(UInt32 index, VideoFrame &out) const {
  Array<UInt8> jpeg;
  if (!readEncodedFrame(index, jpeg)) {
    return false;
  }

  int width = 0;
  int height = 0;
  int channels = 0;
  UInt8 *decoded =
      stbi_load_from_memory(jpeg.data(), static_cast<int>(jpeg.size()), &width,
                            &height, &channels, 4);
  if (!decoded) {
    return false;
  }

  out.width = static_cast<UInt32>(width);
  out.height = static_cast<UInt32>(height);
  out.channels = 4;
  out.pixels.resize(static_cast<Size>(width) * height * 4);
  std::memcpy(out.pixels.data(), decoded, out.pixels.size());
  stbi_image_free(decoded);
  return true;
}

StringView VideoReader::lastError() const { return error; }
} // namespace sinen
