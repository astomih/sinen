#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <vector>

namespace sinen::detail {
struct DdsMip {
  std::vector<uint8_t> pixels;
  uint32_t width = 0, height = 0;
};
struct DdsImage {
  std::vector<DdsMip> mips;
  int channels = 4;
};

// Portable 2D DDS decoder for BC1..BC5 UNORM and masked RGB(A). Texture data
// stays in its authored color space; normal-map conventions belong to
// materials.
inline bool decodeDds(const uint8_t *data, size_t size, DdsImage &output) {
  output = {};
  if (!data || size < 128 || std::memcmp(data, "DDS ", 4))
    return false;
  auto u32 = [&](size_t at) {
    return uint32_t(data[at]) | (uint32_t(data[at + 1]) << 8) |
           (uint32_t(data[at + 2]) << 16) | (uint32_t(data[at + 3]) << 24);
  };
  auto fourcc = [](char a, char b, char c, char d) {
    return uint32_t(a) | (uint32_t(b) << 8) | (uint32_t(c) << 16) |
           (uint32_t(d) << 24);
  };
  const uint32_t width = u32(16), height = u32(12);
  uint32_t levels = std::max(u32(28), 1u);
  if (u32(4) != 124 || u32(76) != 32 || !width || !height || width > 16384 ||
      height > 16384 || u32(24) > 1 || (u32(112) & 0xFE00u))
    return false;
  uint32_t maximumLevels = 1;
  for (auto n = std::max(width, height); n > 1; n >>= 1)
    ++maximumLevels;
  if (levels > maximumLevels)
    return false;
  const uint32_t code = u32(84);
  enum Format { Raw, BC1, BC2, BC3, BC4, BC5 } format = Raw;
  uint32_t bits = u32(88);
  std::array<uint32_t, 4> masks{u32(92), u32(96), u32(100), u32(104)};
  size_t offset = 128;
  if (u32(80) & 4) {
    if (code == fourcc('D', 'X', 'T', '1'))
      format = BC1;
    else if (code == fourcc('D', 'X', 'T', '3'))
      format = BC2;
    else if (code == fourcc('D', 'X', 'T', '5'))
      format = BC3;
    else if (code == fourcc('A', 'T', 'I', '1') ||
             code == fourcc('B', 'C', '4', 'U'))
      format = BC4;
    else if (code == fourcc('A', 'T', 'I', '2') ||
             code == fourcc('B', 'C', '5', 'U'))
      format = BC5;
    else if (code == fourcc('D', 'X', '1', '0')) {
      if (size < 148 || u32(132) != 3 || (u32(136) & 4) || u32(140) != 1)
        return false;
      offset = 148;
      switch (u32(128)) {
      case 71:
      case 72:
        format = BC1;
        break;
      case 74:
      case 75:
        format = BC2;
        break;
      case 77:
      case 78:
        format = BC3;
        break;
      case 80:
        format = BC4;
        break;
      case 83:
        format = BC5;
        break;
      case 28:
      case 29:
        bits = 32;
        masks = {0xff, 0xff00, 0xff0000, 0xff000000};
        break;
      case 87:
      case 91:
        bits = 32;
        masks = {0xff0000, 0xff00, 0xff, 0xff000000};
        break;
      default:
        return false;
      }
    } else
      return false;
  } else if (!(u32(80) & 0x40))
    return false;
  if (format == Raw && (bits != 24 && bits != 32))
    return false;
  output.channels = format == BC4 ? 1 : format == BC5 ? 2 : 4;
  auto scalarBlock = [](const uint8_t *block, uint8_t *values) {
    uint8_t table[8]{block[0], block[1]};
    if (table[0] > table[1]) {
      for (int i = 1; i <= 6; ++i)
        table[i + 1] = uint8_t(((7 - i) * table[0] + i * table[1]) / 7);
    } else {
      for (int i = 1; i <= 4; ++i)
        table[i + 1] = uint8_t(((5 - i) * table[0] + i * table[1]) / 5);
      table[6] = 0;
      table[7] = 255;
    }
    uint64_t indices = 0;
    for (int i = 0; i < 6; ++i)
      indices |= uint64_t(block[i + 2]) << (8 * i);
    for (int i = 0; i < 16; ++i)
      values[i] = table[(indices >> (i * 3)) & 7];
  };
  auto colorBlock = [](const uint8_t *block, bool forceFour, uint8_t *values) {
    const uint16_t c0 = uint16_t(block[0]) | (uint16_t(block[1]) << 8);
    const uint16_t c1 = uint16_t(block[2]) | (uint16_t(block[3]) << 8);
    uint8_t table[4][4]{};
    for (int i = 0; i < 2; ++i) {
      const auto c = i ? c1 : c0;
      table[i][0] = uint8_t(((c >> 11) & 31) * 255 / 31);
      table[i][1] = uint8_t(((c >> 5) & 63) * 255 / 63);
      table[i][2] = uint8_t((c & 31) * 255 / 31);
      table[i][3] = 255;
    }
    for (int c = 0; c < 3; ++c) {
      if (c0 > c1 || forceFour) {
        table[2][c] = uint8_t((2 * table[0][c] + table[1][c]) / 3);
        table[3][c] = uint8_t((table[0][c] + 2 * table[1][c]) / 3);
      } else
        table[2][c] = uint8_t((table[0][c] + table[1][c]) / 2);
    }
    table[2][3] = 255;
    table[3][3] = c0 > c1 || forceFour ? 255 : 0;
    const uint32_t indices = uint32_t(block[4]) | (uint32_t(block[5]) << 8) |
                             (uint32_t(block[6]) << 16) |
                             (uint32_t(block[7]) << 24);
    for (int i = 0; i < 16; ++i)
      std::memcpy(values + i * 4, table[(indices >> (i * 2)) & 3], 4);
  };
  uint64_t decodedBytes = 0;
  for (uint32_t level = 0; level < levels; ++level) {
    DdsMip mip;
    mip.width = std::max(1u, width >> level);
    mip.height = std::max(1u, height >> level);
    const uint64_t bytes = uint64_t(mip.width) * mip.height * output.channels;
    decodedBytes += bytes;
    if (decodedBytes > 512ull * 1024 * 1024)
      return false;
    const uint32_t blockSize = format == BC1 || format == BC4 ? 8 : 16;
    uint64_t sourceRow = format == Raw
                             ? uint64_t(mip.width) * (bits / 8)
                             : uint64_t((mip.width + 3) / 4) * blockSize;
    if (format == Raw && level == 0 && (u32(8) & 8)) {
      if (u32(20) < sourceRow)
        return false;
      sourceRow = u32(20);
    }
    const uint64_t encodedBytes =
        sourceRow * (format == Raw ? mip.height : (mip.height + 3) / 4);
    if (offset > size || encodedBytes > size - offset)
      return false;
    mip.pixels.resize(static_cast<size_t>(bytes));
    if (format == Raw) {
      for (uint32_t y = 0; y < mip.height; ++y)
        for (uint32_t x = 0; x < mip.width; ++x) {
          const auto *pixel = data + offset + y * sourceRow + x * (bits / 8);
          uint32_t packed = 0;
          for (uint32_t i = 0; i < bits / 8; ++i)
            packed |= uint32_t(pixel[i]) << (8 * i);
          for (int channel = 0; channel < 4; ++channel) {
            auto mask = masks[channel];
            uint8_t value = channel == 3 ? 255 : 0;
            if (mask) {
              uint32_t shift = 0;
              while (!(mask & 1)) {
                mask >>= 1;
                ++shift;
              }
              value =
                  uint8_t((uint64_t((packed >> shift) & mask) * 255) / mask);
            }
            mip.pixels[(size_t(y) * mip.width + x) * 4 + channel] = value;
          }
        }
    } else {
      for (uint32_t by = 0; by < (mip.height + 3) / 4; ++by)
        for (uint32_t bx = 0; bx < (mip.width + 3) / 4; ++bx) {
          const auto *block = data + offset + by * sourceRow + bx * blockSize;
          uint8_t pixels[64]{}, alpha[16]{}, second[16]{};
          if (format == BC4 || format == BC5) {
            scalarBlock(block, alpha);
            if (format == BC5)
              scalarBlock(block + 8, second);
            for (int i = 0; i < 16; ++i) {
              pixels[i * output.channels] = alpha[i];
              if (format == BC5)
                pixels[i * 2 + 1] = second[i];
            }
          } else {
            colorBlock(block + (format == BC1 ? 0 : 8), format != BC1, pixels);
            if (format == BC3)
              scalarBlock(block, alpha);
            for (int i = 0; i < 16; ++i) {
              if (format == BC2)
                pixels[i * 4 + 3] =
                    uint8_t(((block[i / 2] >> ((i % 2) * 4)) & 15) * 17);
              if (format == BC3)
                pixels[i * 4 + 3] = alpha[i];
            }
          }
          for (uint32_t y = 0; y < 4 && by * 4 + y < mip.height; ++y)
            for (uint32_t x = 0; x < 4 && bx * 4 + x < mip.width; ++x)
              std::memcpy(mip.pixels.data() +
                              ((size_t(by * 4 + y) * mip.width + bx * 4 + x) *
                               output.channels),
                          pixels + (y * 4 + x) * output.channels,
                          output.channels);
        }
    }
    offset += static_cast<size_t>(encodedBytes);
    output.mips.push_back(std::move(mip));
  }
  return true;
}
} // namespace sinen::detail
