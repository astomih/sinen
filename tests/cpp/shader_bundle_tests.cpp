#include <gpu/shader/shader_bundle.hpp>

#include <gtest/gtest.h>

#include <cstring>
#include <filesystem>
#include <fstream>

namespace sinen {
namespace {
void writeU32(Array<char> &data, size_t offset, uint32_t value) {
  data[offset + 0] = static_cast<char>(value & 0xffu);
  data[offset + 1] = static_cast<char>((value >> 8u) & 0xffu);
  data[offset + 2] = static_cast<char>((value >> 16u) & 0xffu);
  data[offset + 3] = static_cast<char>((value >> 24u) & 0xffu);
}

uint32_t readU32(const Array<char> &data, size_t offset) {
  const auto *bytes =
      reinterpret_cast<const unsigned char *>(data.data() + offset);
  return static_cast<uint32_t>(bytes[0]) |
         (static_cast<uint32_t>(bytes[1]) << 8u) |
         (static_cast<uint32_t>(bytes[2]) << 16u) |
         (static_cast<uint32_t>(bytes[3]) << 24u);
}
} // namespace

TEST(ShaderBundle, PacksBinaryV2WithReflection) {
  const char code[] = {'s', 'h', 'a', 'd', 'e', 'r', '\0', 'x'};
  ShaderBundle::PackEntry source;
  source.stage = ShaderStage::Fragment;
  source.format = ShaderFormat::DXIL;
  source.data = code;
  source.size = sizeof(code);
  source.numSamplers = 2;
  source.numUniformBuffers = 1;
  source.resources.push_back(
      {ShaderBundle::ResourceKind::Texture, "albedoTexture", 1});
  source.resources.push_back(
      {ShaderBundle::ResourceKind::UniformBuffer, "FrameData", 0});

  Array<ShaderBundle::PackEntry> entries;
  entries.push_back(std::move(source));
  const auto bundle = ShaderBundle::pack(entries);

  ASSERT_GE(bundle.size(), 32u);
  EXPECT_EQ(StringView(bundle.data(), 4), "SNSB");
  EXPECT_EQ(readU32(bundle, 4), ShaderBundle::formatVersion);
  EXPECT_EQ(readU32(bundle, 8), bundle.size());
  EXPECT_TRUE(ShaderBundle::isBundle(StringView(bundle.data(), bundle.size())));

  auto selected =
      ShaderBundle::select(StringView(bundle.data(), bundle.size()),
                           ShaderStage::Fragment, ShaderFormat::DXIL);
  ASSERT_TRUE(selected.has_value());
  EXPECT_EQ(selected->code, Array<char>(std::begin(code), std::end(code)));
  EXPECT_EQ(selected->numSamplers, 2u);
  EXPECT_EQ(selected->numUniformBuffers, 1u);
  ASSERT_EQ(selected->resources.size(), 2u);
  EXPECT_EQ(selected->resources[0].kind,
            ShaderBundle::ResourceKind::UniformBuffer);
  EXPECT_EQ(selected->resources[0].name, "FrameData");
  EXPECT_EQ(selected->resources[0].slot, 0u);
  EXPECT_EQ(selected->resources[1].kind, ShaderBundle::ResourceKind::Texture);
  EXPECT_EQ(selected->resources[1].name, "albedoTexture");
  EXPECT_EQ(selected->resources[1].slot, 1u);
}

TEST(ShaderBundle, DebugJsonContainsMetadataButNotShaderCode) {
  constexpr StringView code = "private shader bytecode";
  ShaderBundle::PackEntry source;
  source.stage = ShaderStage::Vertex;
  source.format = ShaderFormat::SPIRV;
  source.data = code.data();
  source.size = static_cast<uint32_t>(code.size());
  source.resources.push_back(
      {ShaderBundle::ResourceKind::UniformBuffer, "SceneData", 3});
  Array<ShaderBundle::PackEntry> entries;
  entries.push_back(std::move(source));

  const String json = ShaderBundle::dumpJson(entries);
  EXPECT_NE(json.find("sinen.shader.bundle.debug"), String::npos);
  EXPECT_NE(json.find("SceneData"), String::npos);
  EXPECT_NE(json.find("codeHash"), String::npos);
  EXPECT_EQ(json.find(code), String::npos);
  EXPECT_EQ(json.find("base64"), String::npos);
}

TEST(ShaderBundle, ReadsLegacyJsonV1) {
  constexpr StringView json = R"({
    "type":"sinen.shader.bundle",
    "version":1,
    "entries":[{
      "stage":"vertex",
      "format":"spirv",
      "numUniformBuffers":1,
      "encoding":"base64",
      "code":"YWJj"
    }]
  })";
  auto selected =
      ShaderBundle::select(json, ShaderStage::Vertex, ShaderFormat::SPIRV);
  ASSERT_TRUE(selected.has_value());
  EXPECT_EQ(StringView(selected->code.data(), selected->code.size()), "abc");
  EXPECT_EQ(selected->numUniformBuffers, 1u);
  EXPECT_TRUE(selected->resources.empty());
}

TEST(ShaderBundle, ReadsLegacyBinaryV1) {
  Array<char> bundle(12 + 36 + 3, 0);
  std::memcpy(bundle.data(), "SNSB", 4);
  writeU32(bundle, 4, 1);
  writeU32(bundle, 8, 1);
  writeU32(bundle, 12, static_cast<uint32_t>(ShaderStage::Compute));
  writeU32(bundle, 16, static_cast<uint32_t>(ShaderFormat::SPIRV));
  writeU32(bundle, 20, 48);
  writeU32(bundle, 24, 3);
  writeU32(bundle, 32, 4);
  std::memcpy(bundle.data() + 48, "old", 3);

  auto selected =
      ShaderBundle::select(StringView(bundle.data(), bundle.size()),
                           ShaderStage::Compute, ShaderFormat::SPIRV);
  ASSERT_TRUE(selected.has_value());
  EXPECT_EQ(StringView(selected->code.data(), selected->code.size()), "old");
  EXPECT_EQ(selected->numStorageBuffers, 4u);
}

TEST(ShaderBundle, RejectsOutOfBoundsV2Offsets) {
  constexpr StringView code = "code";
  ShaderBundle::PackEntry source;
  source.data = code.data();
  source.size = static_cast<uint32_t>(code.size());
  Array<ShaderBundle::PackEntry> entries;
  entries.push_back(std::move(source));
  auto bundle = ShaderBundle::pack(entries);
  ASSERT_GE(bundle.size(), 96u);

  writeU32(bundle, 32 + 8, 0xfffffff0u);
  EXPECT_FALSE(ShaderBundle::select(StringView(bundle.data(), bundle.size()),
                                    ShaderStage::Vertex, ShaderFormat::SPIRV)
                   .has_value());
}

TEST(ShaderBundle, BuiltinBundlesUseV2AndContainReflection) {
  const auto root = std::filesystem::path(SINEN_TEST_SOURCE_DIR) / "src" /
                    "gpu" / "shader" / "default";
  const auto readBundle = [&root](const char *name) {
    std::ifstream stream(root / name, std::ios::binary);
    return Array<char>(std::istreambuf_iterator<char>(stream),
                       std::istreambuf_iterator<char>());
  };

  const auto vertexBundle = readBundle("shader.vert.snb");
  ASSERT_GE(vertexBundle.size(), 32u);
  EXPECT_EQ(StringView(vertexBundle.data(), 4), "SNSB");
  EXPECT_EQ(readU32(vertexBundle, 4), ShaderBundle::formatVersion);
  for (const auto format : {ShaderFormat::SPIRV, ShaderFormat::SPIRV_1_3,
                            ShaderFormat::WGSL, ShaderFormat::DXIL}) {
    auto entry = ShaderBundle::select(
        StringView(vertexBundle.data(), vertexBundle.size()),
        ShaderStage::Vertex, format);
    ASSERT_TRUE(entry.has_value());
    ASSERT_EQ(entry->resources.size(), 1u);
    EXPECT_EQ(entry->resources[0].kind,
              ShaderBundle::ResourceKind::UniformBuffer);
    EXPECT_EQ(entry->resources[0].name, "Mat");
  }

  const auto fragmentBundle = readBundle("shader.frag.snb");
  ASSERT_GE(fragmentBundle.size(), 32u);
  for (const auto format : {ShaderFormat::SPIRV, ShaderFormat::SPIRV_1_3,
                            ShaderFormat::WGSL, ShaderFormat::DXIL}) {
    auto entry = ShaderBundle::select(
        StringView(fragmentBundle.data(), fragmentBundle.size()),
        ShaderStage::Fragment, format);
    ASSERT_TRUE(entry.has_value());
    ASSERT_EQ(entry->resources.size(), 1u);
    EXPECT_EQ(entry->resources[0].kind, ShaderBundle::ResourceKind::Texture);
    EXPECT_EQ(entry->resources[0].name, "sampledTexture");
  }
}
} // namespace sinen
