#include <gpu/shader/shader_bundle.hpp>

#include <core/allocator/engine_memory.hpp>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <limits>

namespace sinen {
namespace {
constexpr char kMagic[4] = {'S', 'N', 'S', 'B'};
constexpr uint32_t kLegacyVersion = 1;
constexpr uint32_t kHeaderSizeV1 = 12;
constexpr uint32_t kEntrySizeV1 = 36;
constexpr uint32_t kHeaderSizeV2 = 32;
constexpr uint32_t kEntrySizeV2 = 56;
constexpr uint32_t kResourceSizeV2 = 16;
constexpr uint32_t kCodeAlignment = 16;
constexpr StringView kBundleType = "sinen.shader.bundle";
constexpr StringView kDebugType = "sinen.shader.bundle.debug";
constexpr StringView kEncodingBase64 = "base64";
constexpr StringView kEntriesKey = "entries";

bool readU32(StringView data, size_t offset, uint32_t &value) {
  if (offset > data.size() || data.size() - offset < 4) {
    return false;
  }
  const auto *bytes =
      reinterpret_cast<const unsigned char *>(data.data() + offset);
  value = static_cast<uint32_t>(bytes[0]) |
          (static_cast<uint32_t>(bytes[1]) << 8u) |
          (static_cast<uint32_t>(bytes[2]) << 16u) |
          (static_cast<uint32_t>(bytes[3]) << 24u);
  return true;
}

void writeU32(Array<char> &out, size_t offset, uint32_t value) {
  out[offset + 0] = static_cast<char>(value & 0xffu);
  out[offset + 1] = static_cast<char>((value >> 8u) & 0xffu);
  out[offset + 2] = static_cast<char>((value >> 16u) & 0xffu);
  out[offset + 3] = static_cast<char>((value >> 24u) & 0xffu);
}

bool checkedRange(uint32_t offset, uint32_t size, size_t totalSize) {
  return static_cast<size_t>(offset) <= totalSize &&
         static_cast<size_t>(size) <= totalSize - offset;
}

bool checkedU32(uint64_t value, uint32_t &out) {
  if (value > std::numeric_limits<uint32_t>::max()) {
    return false;
  }
  out = static_cast<uint32_t>(value);
  return true;
}

uint64_t alignUp(uint64_t value, uint32_t alignment) {
  return (value + alignment - 1u) & ~(static_cast<uint64_t>(alignment) - 1u);
}

uint64_t codeHash(const void *data, uint32_t size) {
  constexpr uint64_t offsetBasis = 14695981039346656037ull;
  constexpr uint64_t prime = 1099511628211ull;
  uint64_t hash = offsetBasis;
  const auto *bytes = static_cast<const unsigned char *>(data);
  for (uint32_t i = 0; i < size; ++i) {
    hash ^= bytes[i];
    hash *= prime;
  }
  return hash;
}

StringView trimLeft(StringView data) {
  while (!data.empty()) {
    const unsigned char c = static_cast<unsigned char>(data.front());
    if (c != ' ' && c != '\t' && c != '\r' && c != '\n') {
      break;
    }
    data.remove_prefix(1);
  }
  return data;
}

bool validStage(ShaderStage stage) {
  switch (stage) {
  case ShaderStage::Vertex:
  case ShaderStage::Fragment:
  case ShaderStage::Compute:
  case ShaderStage::RayGeneration:
  case ShaderStage::AnyHit:
  case ShaderStage::ClosestHit:
  case ShaderStage::Miss:
  case ShaderStage::Intersection:
  case ShaderStage::Callable:
    return true;
  }
  return false;
}

bool validFormat(ShaderFormat format) {
  switch (format) {
  case ShaderFormat::SPIRV:
  case ShaderFormat::WGSL:
  case ShaderFormat::DXBC:
  case ShaderFormat::DXIL:
  case ShaderFormat::SPIRV_1_3:
    return true;
  }
  return false;
}

const char *stageToString(ShaderStage stage) {
  switch (stage) {
  case ShaderStage::Vertex:
    return "vertex";
  case ShaderStage::Fragment:
    return "fragment";
  case ShaderStage::Compute:
    return "compute";
  case ShaderStage::RayGeneration:
    return "rayGeneration";
  case ShaderStage::AnyHit:
    return "anyHit";
  case ShaderStage::ClosestHit:
    return "closestHit";
  case ShaderStage::Miss:
    return "miss";
  case ShaderStage::Intersection:
    return "intersection";
  case ShaderStage::Callable:
    return "callable";
  }
  return "unknown";
}

bool stageFromString(StringView value, ShaderStage &stage) {
  if (value == "vertex") {
    stage = ShaderStage::Vertex;
  } else if (value == "fragment") {
    stage = ShaderStage::Fragment;
  } else if (value == "compute") {
    stage = ShaderStage::Compute;
  } else if (value == "rayGeneration") {
    stage = ShaderStage::RayGeneration;
  } else if (value == "anyHit") {
    stage = ShaderStage::AnyHit;
  } else if (value == "closestHit") {
    stage = ShaderStage::ClosestHit;
  } else if (value == "miss") {
    stage = ShaderStage::Miss;
  } else if (value == "intersection") {
    stage = ShaderStage::Intersection;
  } else if (value == "callable") {
    stage = ShaderStage::Callable;
  } else {
    return false;
  }
  return true;
}

const char *formatToString(ShaderFormat format) {
  switch (format) {
  case ShaderFormat::SPIRV:
    return "spirv";
  case ShaderFormat::WGSL:
    return "wgsl";
  case ShaderFormat::DXBC:
    return "dxbc";
  case ShaderFormat::DXIL:
    return "dxil";
  case ShaderFormat::SPIRV_1_3:
    return "spirv_1_3";
  }
  return "unknown";
}

bool formatFromString(StringView value, ShaderFormat &format) {
  if (value == "spirv") {
    format = ShaderFormat::SPIRV;
  } else if (value == "wgsl") {
    format = ShaderFormat::WGSL;
  } else if (value == "dxbc") {
    format = ShaderFormat::DXBC;
  } else if (value == "dxil") {
    format = ShaderFormat::DXIL;
  } else if (value == "spirv_1_3") {
    format = ShaderFormat::SPIRV_1_3;
  } else {
    return false;
  }
  return true;
}

const char *resourceKindToString(ShaderBundle::ResourceKind kind) {
  switch (kind) {
  case ShaderBundle::ResourceKind::UniformBuffer:
    return "uniformBuffer";
  case ShaderBundle::ResourceKind::Texture:
    return "texture";
  }
  return "unknown";
}

bool validResourceKind(ShaderBundle::ResourceKind kind) {
  return kind == ShaderBundle::ResourceKind::UniformBuffer ||
         kind == ShaderBundle::ResourceKind::Texture;
}

Array<ShaderBundle::ResourceBinding>
sortedResources(const ShaderBundle::PackEntry &entry) {
  Array<ShaderBundle::ResourceBinding> resources = entry.resources;
  std::sort(
      resources.begin(), resources.end(), [](const auto &a, const auto &b) {
        if (a.kind != b.kind) {
          return static_cast<uint32_t>(a.kind) < static_cast<uint32_t>(b.kind);
        }
        if (a.slot != b.slot) {
          return a.slot < b.slot;
        }
        return a.name < b.name;
      });
  return resources;
}

void addStringMember(rapidjson::Document &doc, rapidjson::Value &object,
                     StringView key, StringView value) {
  auto &allocator = doc.GetAllocator();
  rapidjson::Value jsonKey;
  rapidjson::Value jsonValue;
  jsonKey.SetString(key.data(), static_cast<rapidjson::SizeType>(key.size()),
                    allocator);
  jsonValue.SetString(
      value.data(), static_cast<rapidjson::SizeType>(value.size()), allocator);
  object.AddMember(jsonKey, jsonValue, allocator);
}

int base64Value(char c) {
  if (c >= 'A' && c <= 'Z') {
    return c - 'A';
  }
  if (c >= 'a' && c <= 'z') {
    return c - 'a' + 26;
  }
  if (c >= '0' && c <= '9') {
    return c - '0' + 52;
  }
  if (c == '+') {
    return 62;
  }
  if (c == '/') {
    return 63;
  }
  return -1;
}

bool base64Decode(StringView encoded, Array<char> &out) {
  out.clear();
  if ((encoded.size() % 4) != 0) {
    return false;
  }
  if (encoded.empty()) {
    return true;
  }

  size_t padding = 0;
  if (encoded[encoded.size() - 1] == '=') {
    ++padding;
  }
  if (encoded.size() >= 2 && encoded[encoded.size() - 2] == '=') {
    ++padding;
  }
  out.reserve((encoded.size() / 4) * 3 - padding);

  for (size_t i = 0; i < encoded.size(); i += 4) {
    const bool pad2 = encoded[i + 2] == '=';
    const bool pad3 = encoded[i + 3] == '=';
    if (encoded[i] == '=' || encoded[i + 1] == '=' || (pad2 && !pad3) ||
        (i + 4 != encoded.size() && (pad2 || pad3))) {
      return false;
    }
    const int v0 = base64Value(encoded[i]);
    const int v1 = base64Value(encoded[i + 1]);
    const int v2 = pad2 ? 0 : base64Value(encoded[i + 2]);
    const int v3 = pad3 ? 0 : base64Value(encoded[i + 3]);
    if (v0 < 0 || v1 < 0 || v2 < 0 || v3 < 0) {
      return false;
    }
    const uint32_t triple = (static_cast<uint32_t>(v0) << 18u) |
                            (static_cast<uint32_t>(v1) << 12u) |
                            (static_cast<uint32_t>(v2) << 6u) |
                            static_cast<uint32_t>(v3);
    out.push_back(static_cast<char>((triple >> 16u) & 0xffu));
    if (!pad2) {
      out.push_back(static_cast<char>((triple >> 8u) & 0xffu));
    }
    if (!pad3) {
      out.push_back(static_cast<char>(triple & 0xffu));
    }
  }
  return true;
}

bool readRequiredUint(const rapidjson::Value &object, const char *name,
                      uint32_t &value) {
  if (!object.HasMember(name) || !object[name].IsUint()) {
    return false;
  }
  value = object[name].GetUint();
  return true;
}

bool readOptionalUint(const rapidjson::Value &object, const char *name,
                      uint32_t &value) {
  if (!object.HasMember(name)) {
    value = 0;
    return true;
  }
  if (!object[name].IsUint()) {
    return false;
  }
  value = object[name].GetUint();
  return true;
}

bool parseLegacyJsonBundle(StringView data, rapidjson::Document &doc) {
  doc.Parse(data.data(), data.size());
  if (doc.HasParseError() || !doc.IsObject()) {
    return false;
  }
  if (!doc.HasMember("type") || !doc["type"].IsString() ||
      StringView(doc["type"].GetString(), doc["type"].GetStringLength()) !=
          kBundleType) {
    return false;
  }
  uint32_t version = 0;
  if (!readRequiredUint(doc, "version", version) || version != kLegacyVersion) {
    return false;
  }
  return doc.HasMember(kEntriesKey.data()) && doc[kEntriesKey.data()].IsArray();
}

std::optional<ShaderBundle::Entry>
selectLegacyBinaryBundle(StringView data, ShaderStage stage,
                         ShaderFormat preferredFormat) {
  uint32_t version = 0;
  uint32_t entryCount = 0;
  if (!readU32(data, 4, version) || !readU32(data, 8, entryCount) ||
      version != kLegacyVersion) {
    return std::nullopt;
  }
  const uint64_t tableEnd =
      kHeaderSizeV1 + static_cast<uint64_t>(entryCount) * kEntrySizeV1;
  if (tableEnd > data.size()) {
    return std::nullopt;
  }

  for (uint32_t i = 0; i < entryCount; ++i) {
    const size_t base = kHeaderSizeV1 + static_cast<size_t>(i) * kEntrySizeV1;
    uint32_t rawStage = 0;
    uint32_t rawFormat = 0;
    uint32_t offset = 0;
    uint32_t size = 0;
    ShaderBundle::Entry entry;
    if (!readU32(data, base + 0, rawStage) ||
        !readU32(data, base + 4, rawFormat) ||
        !readU32(data, base + 8, offset) || !readU32(data, base + 12, size) ||
        !readU32(data, base + 16, entry.numSamplers) ||
        !readU32(data, base + 20, entry.numStorageBuffers) ||
        !readU32(data, base + 24, entry.numStorageTextures) ||
        !readU32(data, base + 28, entry.numUniformBuffers)) {
      return std::nullopt;
    }
    entry.stage = static_cast<ShaderStage>(rawStage);
    entry.format = static_cast<ShaderFormat>(rawFormat);
    if (!validStage(entry.stage) || !validFormat(entry.format) ||
        !checkedRange(offset, size, data.size())) {
      return std::nullopt;
    }
    if (entry.stage != stage || entry.format != preferredFormat) {
      continue;
    }
    entry.code.resize(size);
    if (size > 0) {
      std::memcpy(entry.code.data(), data.data() + offset, size);
    }
    return entry;
  }
  return std::nullopt;
}

std::optional<ShaderBundle::Entry>
selectBinaryBundleV2(StringView data, ShaderStage stage,
                     ShaderFormat preferredFormat) {
  uint32_t fileSize = 0;
  uint32_t entryCount = 0;
  uint32_t entryTableOffset = 0;
  uint32_t stringTableOffset = 0;
  uint32_t stringTableSize = 0;
  uint32_t entryTableSize = 0;
  if (!readU32(data, 8, fileSize) || !readU32(data, 12, entryCount) ||
      !readU32(data, 16, entryTableOffset) ||
      !readU32(data, 20, stringTableOffset) ||
      !readU32(data, 24, stringTableSize) ||
      !checkedU32(static_cast<uint64_t>(entryCount) * kEntrySizeV2,
                  entryTableSize) ||
      fileSize != data.size() || entryTableOffset < kHeaderSizeV2 ||
      !checkedRange(entryTableOffset, entryTableSize, data.size()) ||
      !checkedRange(stringTableOffset, stringTableSize, data.size())) {
    return std::nullopt;
  }
  const uint64_t entryTableEnd =
      static_cast<uint64_t>(entryTableOffset) + entryTableSize;
  const uint64_t stringTableEnd =
      static_cast<uint64_t>(stringTableOffset) + stringTableSize;
  if (stringTableOffset < entryTableEnd || stringTableEnd > data.size()) {
    return std::nullopt;
  }

  for (uint32_t i = 0; i < entryCount; ++i) {
    const size_t base =
        entryTableOffset + static_cast<size_t>(i) * kEntrySizeV2;
    uint32_t rawStage = 0;
    uint32_t rawFormat = 0;
    uint32_t codeOffset = 0;
    uint32_t codeSize = 0;
    uint32_t resourceOffset = 0;
    uint32_t resourceCount = 0;
    ShaderBundle::Entry entry;
    if (!readU32(data, base + 0, rawStage) ||
        !readU32(data, base + 4, rawFormat) ||
        !readU32(data, base + 8, codeOffset) ||
        !readU32(data, base + 12, codeSize) ||
        !readU32(data, base + 16, resourceOffset) ||
        !readU32(data, base + 20, resourceCount) ||
        !readU32(data, base + 24, entry.numSamplers) ||
        !readU32(data, base + 28, entry.numStorageBuffers) ||
        !readU32(data, base + 32, entry.numStorageTextures) ||
        !readU32(data, base + 36, entry.numUniformBuffers)) {
      return std::nullopt;
    }
    entry.stage = static_cast<ShaderStage>(rawStage);
    entry.format = static_cast<ShaderFormat>(rawFormat);
    uint32_t resourceBytes = 0;
    if (!validStage(entry.stage) || !validFormat(entry.format) ||
        !checkedRange(codeOffset, codeSize, data.size()) ||
        codeOffset < stringTableEnd || (codeOffset % kCodeAlignment) != 0 ||
        !checkedU32(static_cast<uint64_t>(resourceCount) * kResourceSizeV2,
                    resourceBytes) ||
        resourceOffset < entryTableEnd || resourceOffset > stringTableOffset ||
        resourceBytes > stringTableOffset - resourceOffset) {
      return std::nullopt;
    }
    if (entry.stage != stage || entry.format != preferredFormat) {
      continue;
    }

    entry.resources.reserve(resourceCount);
    for (uint32_t resourceIndex = 0; resourceIndex < resourceCount;
         ++resourceIndex) {
      const size_t resourceBase =
          resourceOffset + static_cast<size_t>(resourceIndex) * kResourceSizeV2;
      uint32_t rawKind = 0;
      uint32_t slot = 0;
      uint32_t nameOffset = 0;
      uint32_t nameSize = 0;
      if (!readU32(data, resourceBase + 0, rawKind) ||
          !readU32(data, resourceBase + 4, slot) ||
          !readU32(data, resourceBase + 8, nameOffset) ||
          !readU32(data, resourceBase + 12, nameSize) ||
          nameOffset > stringTableSize ||
          nameSize > stringTableSize - nameOffset) {
        return std::nullopt;
      }
      const auto kind = static_cast<ShaderBundle::ResourceKind>(rawKind);
      if (!validResourceKind(kind)) {
        return std::nullopt;
      }
      entry.resources.push_back(
          {kind, String(data.data() + stringTableOffset + nameOffset, nameSize),
           slot});
    }
    entry.code.resize(codeSize);
    if (codeSize > 0) {
      std::memcpy(entry.code.data(), data.data() + codeOffset, codeSize);
    }
    return entry;
  }
  return std::nullopt;
}

std::optional<ShaderBundle::Entry>
selectLegacyJsonBundle(StringView data, ShaderStage stage,
                       ShaderFormat preferredFormat) {
  rapidjson::Document doc;
  if (!parseLegacyJsonBundle(data, doc)) {
    return std::nullopt;
  }
  const auto &entries = doc[kEntriesKey.data()];
  for (const auto &jsonEntry : entries.GetArray()) {
    if (!jsonEntry.IsObject() || !jsonEntry.HasMember("stage") ||
        !jsonEntry["stage"].IsString() || !jsonEntry.HasMember("format") ||
        !jsonEntry["format"].IsString() || !jsonEntry.HasMember("encoding") ||
        !jsonEntry["encoding"].IsString() || !jsonEntry.HasMember("code") ||
        !jsonEntry["code"].IsString()) {
      return std::nullopt;
    }
    ShaderBundle::Entry entry;
    if (!stageFromString(StringView(jsonEntry["stage"].GetString(),
                                    jsonEntry["stage"].GetStringLength()),
                         entry.stage) ||
        !formatFromString(StringView(jsonEntry["format"].GetString(),
                                     jsonEntry["format"].GetStringLength()),
                          entry.format) ||
        !readOptionalUint(jsonEntry, "numSamplers", entry.numSamplers) ||
        !readOptionalUint(jsonEntry, "numStorageBuffers",
                          entry.numStorageBuffers) ||
        !readOptionalUint(jsonEntry, "numStorageTextures",
                          entry.numStorageTextures) ||
        !readOptionalUint(jsonEntry, "numUniformBuffers",
                          entry.numUniformBuffers)) {
      return std::nullopt;
    }
    if (entry.stage != stage || entry.format != preferredFormat) {
      continue;
    }
    if (StringView(jsonEntry["encoding"].GetString(),
                   jsonEntry["encoding"].GetStringLength()) !=
            kEncodingBase64 ||
        !base64Decode(StringView(jsonEntry["code"].GetString(),
                                 jsonEntry["code"].GetStringLength()),
                      entry.code)) {
      return std::nullopt;
    }
    return entry;
  }
  return std::nullopt;
}
} // namespace

bool ShaderBundle::isBundle(StringView data) {
  if (data.size() >= kHeaderSizeV1 &&
      std::memcmp(data.data(), kMagic, sizeof(kMagic)) == 0) {
    uint32_t version = 0;
    return readU32(data, 4, version) &&
           (version == kLegacyVersion || version == formatVersion);
  }
  data = trimLeft(data);
  if (data.empty() || data.front() != '{') {
    return false;
  }
  rapidjson::Document doc;
  return parseLegacyJsonBundle(data, doc);
}

Array<char> ShaderBundle::pack(const Array<PackEntry> &entries) {
  uint32_t entryCount = 0;
  if (!checkedU32(entries.size(), entryCount)) {
    return {};
  }

  struct BuildEntry {
    const PackEntry *source = nullptr;
    Array<ResourceBinding> resources;
    uint32_t resourceOffset = 0;
    uint32_t codeOffset = 0;
  };
  Array<BuildEntry> buildEntries;
  buildEntries.reserve(entries.size());

  uint64_t totalResources = 0;
  uint64_t stringTableSize64 = 0;
  for (const auto &entry : entries) {
    if (!validStage(entry.stage) || !validFormat(entry.format) ||
        (entry.size > 0 && entry.data == nullptr)) {
      return {};
    }
    auto resources = sortedResources(entry);
    if (resources.size() > std::numeric_limits<uint32_t>::max()) {
      return {};
    }
    for (const auto &resource : resources) {
      uint32_t nameSize = 0;
      if (!validResourceKind(resource.kind) ||
          !checkedU32(resource.name.size(), nameSize)) {
        return {};
      }
      stringTableSize64 += resource.name.size();
    }
    totalResources += resources.size();
    buildEntries.push_back({&entry, std::move(resources), 0, 0});
  }

  const uint64_t entryTableOffset64 = kHeaderSizeV2;
  const uint64_t resourceTableOffset64 =
      entryTableOffset64 + static_cast<uint64_t>(entryCount) * kEntrySizeV2;
  const uint64_t stringTableOffset64 =
      resourceTableOffset64 + totalResources * kResourceSizeV2;
  uint64_t cursor =
      alignUp(stringTableOffset64 + stringTableSize64, kCodeAlignment);
  for (auto &entry : buildEntries) {
    if (!checkedU32(cursor, entry.codeOffset)) {
      return {};
    }
    cursor = alignUp(cursor + entry.source->size, kCodeAlignment);
  }

  uint32_t fileSize = 0;
  uint32_t entryTableOffset = 0;
  uint32_t stringTableOffset = 0;
  uint32_t stringTableSize = 0;
  if (!checkedU32(cursor, fileSize) ||
      !checkedU32(entryTableOffset64, entryTableOffset) ||
      !checkedU32(stringTableOffset64, stringTableOffset) ||
      !checkedU32(stringTableSize64, stringTableSize)) {
    return {};
  }

  Array<char> out;
  out.resize(fileSize, 0);
  std::memcpy(out.data(), kMagic, sizeof(kMagic));
  writeU32(out, 4, formatVersion);
  writeU32(out, 8, fileSize);
  writeU32(out, 12, entryCount);
  writeU32(out, 16, entryTableOffset);
  writeU32(out, 20, stringTableOffset);
  writeU32(out, 24, stringTableSize);
  writeU32(out, 28, 0);

  uint32_t resourceCursor = static_cast<uint32_t>(resourceTableOffset64);
  uint32_t stringCursor = 0;
  for (size_t i = 0; i < buildEntries.size(); ++i) {
    auto &buildEntry = buildEntries[i];
    const auto &entry = *buildEntry.source;
    buildEntry.resourceOffset = resourceCursor;
    const size_t base = entryTableOffset + i * kEntrySizeV2;
    writeU32(out, base + 0, static_cast<uint32_t>(entry.stage));
    writeU32(out, base + 4, static_cast<uint32_t>(entry.format));
    writeU32(out, base + 8, buildEntry.codeOffset);
    writeU32(out, base + 12, entry.size);
    writeU32(out, base + 16, buildEntry.resourceOffset);
    writeU32(out, base + 20,
             static_cast<uint32_t>(buildEntry.resources.size()));
    writeU32(out, base + 24, entry.numSamplers);
    writeU32(out, base + 28, entry.numStorageBuffers);
    writeU32(out, base + 32, entry.numStorageTextures);
    writeU32(out, base + 36, entry.numUniformBuffers);
    const uint64_t hash = codeHash(entry.data, entry.size);
    writeU32(out, base + 40, static_cast<uint32_t>(hash));
    writeU32(out, base + 44, static_cast<uint32_t>(hash >> 32u));
    writeU32(out, base + 48, 0);
    writeU32(out, base + 52, 0);

    for (const auto &resource : buildEntry.resources) {
      writeU32(out, resourceCursor + 0, static_cast<uint32_t>(resource.kind));
      writeU32(out, resourceCursor + 4, resource.slot);
      writeU32(out, resourceCursor + 8, stringCursor);
      writeU32(out, resourceCursor + 12,
               static_cast<uint32_t>(resource.name.size()));
      if (!resource.name.empty()) {
        std::memcpy(out.data() + stringTableOffset + stringCursor,
                    resource.name.data(), resource.name.size());
      }
      stringCursor += static_cast<uint32_t>(resource.name.size());
      resourceCursor += kResourceSizeV2;
    }
    if (entry.size > 0) {
      std::memcpy(out.data() + buildEntry.codeOffset, entry.data, entry.size);
    }
  }
  return out;
}

Buffer ShaderBundle::packBuffer(const Array<PackEntry> &entries) {
  auto bundle = pack(entries);
  Buffer buffer =
      makeBuffer(bundle.size(), BufferType::Binary, EngineMemory::asset());
  if (!bundle.empty()) {
    std::memcpy(buffer.data(), bundle.data(), bundle.size());
  }
  return buffer;
}

String ShaderBundle::dumpJson(const Array<PackEntry> &entries) {
  rapidjson::Document doc;
  doc.SetObject();
  auto &allocator = doc.GetAllocator();
  addStringMember(doc, doc, "type", kDebugType);
  doc.AddMember("version", formatVersion, allocator);

  rapidjson::Value jsonEntries(rapidjson::kArrayType);
  for (const auto &entry : entries) {
    if (!validStage(entry.stage) || !validFormat(entry.format) ||
        (entry.size > 0 && entry.data == nullptr)) {
      return {};
    }
    rapidjson::Value jsonEntry(rapidjson::kObjectType);
    addStringMember(doc, jsonEntry, "stage", stageToString(entry.stage));
    addStringMember(doc, jsonEntry, "format", formatToString(entry.format));
    jsonEntry.AddMember("codeSize", entry.size, allocator);
    char hashText[17]{};
    std::snprintf(
        hashText, sizeof(hashText), "%016llx",
        static_cast<unsigned long long>(codeHash(entry.data, entry.size)));
    addStringMember(doc, jsonEntry, "codeHash", hashText);
    jsonEntry.AddMember("numSamplers", entry.numSamplers, allocator);
    jsonEntry.AddMember("numStorageBuffers", entry.numStorageBuffers,
                        allocator);
    jsonEntry.AddMember("numStorageTextures", entry.numStorageTextures,
                        allocator);
    jsonEntry.AddMember("numUniformBuffers", entry.numUniformBuffers,
                        allocator);

    rapidjson::Value jsonResources(rapidjson::kArrayType);
    for (const auto &resource : sortedResources(entry)) {
      if (!validResourceKind(resource.kind)) {
        return {};
      }
      rapidjson::Value jsonResource(rapidjson::kObjectType);
      addStringMember(doc, jsonResource, "kind",
                      resourceKindToString(resource.kind));
      addStringMember(doc, jsonResource, "name", resource.name);
      jsonResource.AddMember("slot", resource.slot, allocator);
      jsonResources.PushBack(jsonResource, allocator);
    }
    jsonEntry.AddMember("resources", jsonResources, allocator);
    jsonEntries.PushBack(jsonEntry, allocator);
  }
  doc.AddMember("entries", jsonEntries, allocator);

  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
  writer.SetIndent(' ', 2);
  doc.Accept(writer);
  return String(buffer.GetString(), buffer.GetSize());
}

std::optional<ShaderBundle::Entry>
ShaderBundle::select(StringView data, ShaderStage stage,
                     ShaderFormat preferredFormat) {
  if (!isBundle(data)) {
    return std::nullopt;
  }
  if (data.size() >= kHeaderSizeV1 &&
      std::memcmp(data.data(), kMagic, sizeof(kMagic)) == 0) {
    uint32_t version = 0;
    if (!readU32(data, 4, version)) {
      return std::nullopt;
    }
    if (version == formatVersion) {
      return selectBinaryBundleV2(data, stage, preferredFormat);
    }
    return selectLegacyBinaryBundle(data, stage, preferredFormat);
  }
  return selectLegacyJsonBundle(trimLeft(data), stage, preferredFormat);
}

ShaderFormat ShaderBundle::preferredFormatFor(GPUBackendAPI backendAPI) {
  switch (backendAPI) {
  case GPUBackendAPI::WebGPU:
    return ShaderFormat::WGSL;
  case GPUBackendAPI::SDLGPU:
    return ShaderFormat::SPIRV_1_3;
#ifdef SINEN_PLATFORM_WINDOWS
  case GPUBackendAPI::D3D12:
    return ShaderFormat::DXIL;
#endif
  default:
    return ShaderFormat::SPIRV;
  }
}

const char *ShaderBundle::entryPointFor(ShaderStage stage,
                                        ShaderFormat format) {
  if (format == ShaderFormat::WGSL) {
    switch (stage) {
    case ShaderStage::Vertex:
      return "VSMain";
    case ShaderStage::Fragment:
      return "FSMain";
    case ShaderStage::Compute:
      return "CSMain";
    case ShaderStage::RayGeneration:
      return "RayGenMain";
    case ShaderStage::AnyHit:
      return "AnyHitMain";
    case ShaderStage::ClosestHit:
      return "ClosestHitMain";
    case ShaderStage::Miss:
      return "MissMain";
    case ShaderStage::Intersection:
      return "IntersectionMain";
    case ShaderStage::Callable:
      return "CallableMain";
    }
  }
  return "main";
}
} // namespace sinen
