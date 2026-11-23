#include "graphics/texture/dds_decoder.hpp"
#include <gtest/gtest.h>

namespace {
std::vector<uint8_t> dds(const char *code, uint32_t w, uint32_t h, size_t bytes,
                         uint32_t levels = 1) {
  std::vector<uint8_t> data(128 + bytes);
  auto word = [&](size_t offset, uint32_t value) {
    for (int b = 0; b < 4; ++b)
      data[offset + b] = uint8_t(value >> (8 * b));
  };
  std::memcpy(data.data(), "DDS ", 4);
  word(4, 124);
  word(12, h);
  word(16, w);
  word(28, levels);
  word(76, 32);
  word(80, 4);
  std::memcpy(data.data() + 84, code, 4);
  return data;
}
TEST(DdsDecoder, ClipsBc1EdgeBlocksAndPreservesMipLevels) {
  auto data = dds("DXT1", 3, 2, 16, 2);
  data[128] = data[136] = 0;
  data[129] = data[137] = 0xf8;
  sinen::detail::DdsImage image;
  ASSERT_TRUE(decodeDds(data.data(), data.size(), image));
  ASSERT_EQ(image.mips.size(), 2u);
  EXPECT_EQ(image.mips[0].pixels.size(), 3 * 2 * 4);
  EXPECT_EQ(image.mips[1].width, 1u);
  EXPECT_EQ(image.mips[1].height, 1u);
  for (const auto &mip : image.mips)
    for (size_t i = 0; i < mip.pixels.size(); i += 4) {
      EXPECT_EQ(mip.pixels[i], 255);
      EXPECT_EQ(mip.pixels[i + 1], 0);
      EXPECT_EQ(mip.pixels[i + 2], 0);
      EXPECT_EQ(mip.pixels[i + 3], 255);
    }
  EXPECT_FALSE(decodeDds(data.data(), data.size() - 1, image));
}
TEST(DdsDecoder, DecodesBc1TransparencyAndBc3Alpha) {
  auto data = dds("DXT1", 4, 4, 8);
  std::fill(data.begin() + 132, data.end(), 0xff);
  sinen::detail::DdsImage image;
  ASSERT_TRUE(decodeDds(data.data(), data.size(), image));
  EXPECT_EQ(image.mips[0].pixels[3], 0);
  data = dds("DXT5", 4, 4, 16);
  data[128] = 128;
  data[129] = 64;
  data[136] = 0xe0;
  data[137] = 0x07;
  ASSERT_TRUE(decodeDds(data.data(), data.size(), image));
  EXPECT_EQ(image.mips[0].pixels[1], 255);
  EXPECT_EQ(image.mips[0].pixels[3], 128);
}
TEST(DdsDecoder, KeepsBc5NormalComponentsAndRejectsInvalidHeaders) {
  auto data = dds("ATI2", 4, 4, 16);
  data[128] = 128;
  data[136] = 192;
  sinen::detail::DdsImage image;
  ASSERT_TRUE(decodeDds(data.data(), data.size(), image));
  EXPECT_EQ(image.channels, 2);
  EXPECT_EQ(image.mips[0].pixels[0], 128);
  EXPECT_EQ(image.mips[0].pixels[1], 192);
  data[113] = 2; // Cubemap bit.
  EXPECT_FALSE(decodeDds(data.data(), data.size(), image));
  data = dds("DXT1", 0, 4, 8);
  EXPECT_FALSE(decodeDds(data.data(), data.size(), image));
  data = dds("DXT1", 4, 4, 32, 4); // Only three mips are valid.
  EXPECT_FALSE(decodeDds(data.data(), data.size(), image));
}
} // namespace
