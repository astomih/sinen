#include <gpu/shader/shader_bundle.hpp>

#include <core/allocator/engine_memory.hpp>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <cstring>
#include <limits>

namespace sinen {
namespace {
constexpr char kMagic[4] = {'S', 'N', 'S', 'B'};
constexpr uint32_t kVersion = 1;
constexpr uint32_t kHeaderSize = 12;
// stage, format, code offset/size, sampler/storage/uniform counts, reserved.
constexpr uint32_t kEntrySize = 36;
constexpr StringView kBundleType = "sinen.shader.bundle";
constexpr StringView kEncodingBase64 = "base64";
constexpr StringView kEntriesKey = "entries";

constexpr char kBase64Chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

bool readU32(StringView data, size_t offset, uint32_t &value) {
  if (offset + 4 > data.size()) {
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

void appendBytes(Array<char> &out, const void *data, uint32_t size) {
  if (size == 0) {
    return;
  }
  const auto *bytes = static_cast<const char *>(data);
  out.insert(out.end(), bytes, bytes + size);
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
  return "vertex";
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
  return "spirv";
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

bool addStringMember(rapidjson::Document &doc, rapidjson::Value &object,
                     StringView key, StringView value) {
  auto &allocator = doc.GetAllocator();
  rapidjson::Value jsonKey;
  rapidjson::Value jsonValue;
  jsonKey.SetString(key.data(), static_cast<rapidjson::SizeType>(key.size()),
                    allocator);
  jsonValue.SetString(value.data(),
                      static_cast<rapidjson::SizeType>(value.size()),
                      allocator);
  object.AddMember(jsonKey, jsonValue, allocator);
  return true;
}

String base64Encode(const void *data, uint32_t size) {
  String out;
  if (size == 0) {
    return out;
  }

  const auto *bytes = static_cast<const unsigned char *>(data);
  out.reserve(((static_cast<size_t>(size) + 2) / 3) * 4);
  for (uint32_t i = 0; i < size; i += 3) {
    const uint32_t remaining = size - i;
    const uint32_t b0 = bytes[i];
    const uint32_t b1 = remaining > 1 ? bytes[i + 1] : 0;
    const uint32_t b2 = remaining > 2 ? bytes[i + 2] : 0;
    const uint32_t triple = (b0 << 16u) | (b1 << 8u) | b2;
    out.push_back(kBase64Chars[(triple >> 18u) & 0x3fu]);
    out.push_back(kBase64Chars[(triple >> 12u) & 0x3fu]);
    out.push_back(remaining > 1 ? kBase64Chars[(triple >> 6u) & 0x3fu] : '=');
    out.push_back(remaining > 2 ? kBase64Chars[triple & 0x3fu] : '=');
  }
  return out;
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
  if (encoded.size() >= 1 && encoded[encoded.size() - 1] == '=') {
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

bool parseJsonBundle(StringView data, rapidjson::Document &doc) {
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
  if (!readRequiredUint(doc, "version", version) || version != kVersion) {
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
      version != kVersion) {
    return std::nullopt;
  }

  const size_t tableEnd =
      kHeaderSize + static_cast<size_t>(entryCount) * kEntrySize;
  if (tableEnd > data.size()) {
    return std::nullopt;
  }

  for (uint32_t i = 0; i < entryCount; ++i) {
    const size_t base = kHeaderSize + static_cast<size_t>(i) * kEntrySize;
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
    if (entry.stage != stage || entry.format != preferredFormat) {
      continue;
    }
    if (static_cast<size_t>(offset) + static_cast<size_t>(size) > data.size()) {
      return std::nullopt;
    }
    entry.code.resize(size);
    if (size > 0) {
      std::memcpy(entry.code.data(), data.data() + offset, size);
    }
    return entry;
  }

  return std::nullopt;
}

} // namespace

bool ShaderBundle::isBundle(StringView data) {
  if (data.size() >= kHeaderSize && std::memcmp(data.data(), kMagic, 4) == 0) {
    return true;
  }
  data = trimLeft(data);
  if (data.empty() || data.front() != '{') {
    return false;
  }
  rapidjson::Document doc;
  return parseJsonBundle(data, doc);
}

Array<char> ShaderBundle::pack(const Array<PackEntry> &entries) {
  if (entries.size() > std::numeric_limits<uint32_t>::max()) {
    return {};
  }

  rapidjson::Document doc;
  doc.SetObject();
  auto &allocator = doc.GetAllocator();

  addStringMember(doc, doc, "type", kBundleType);
  doc.AddMember("version", kVersion, allocator);

  rapidjson::Value jsonEntries(rapidjson::kArrayType);
  jsonEntries.Reserve(static_cast<rapidjson::SizeType>(entries.size()),
                      allocator);
  for (const auto &entry : entries) {
    if (entry.size > 0 && entry.data == nullptr) {
      return {};
    }

    rapidjson::Value jsonEntry(rapidjson::kObjectType);
    addStringMember(doc, jsonEntry, "stage", stageToString(entry.stage));
    addStringMember(doc, jsonEntry, "format", formatToString(entry.format));
    jsonEntry.AddMember("numSamplers", entry.numSamplers, allocator);
    jsonEntry.AddMember("numStorageBuffers", entry.numStorageBuffers,
                        allocator);
    jsonEntry.AddMember("numStorageTextures", entry.numStorageTextures,
                        allocator);
    jsonEntry.AddMember("numUniformBuffers", entry.numUniformBuffers,
                        allocator);
    addStringMember(doc, jsonEntry, "encoding", kEncodingBase64);
    addStringMember(doc, jsonEntry, "code",
                    base64Encode(entry.data, entry.size));
    jsonEntries.PushBack(jsonEntry, allocator);
  }
  doc.AddMember("entries", jsonEntries, allocator);

  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  doc.Accept(writer);

  Array<char> out;
  if (buffer.GetSize() > std::numeric_limits<uint32_t>::max()) {
    return {};
  }
  appendBytes(out, buffer.GetString(), static_cast<uint32_t>(buffer.GetSize()));
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

std::optional<ShaderBundle::Entry>
ShaderBundle::select(StringView data, ShaderStage stage,
                     ShaderFormat preferredFormat) {
  if (!isBundle(data)) {
    return std::nullopt;
  }

  if (data.size() >= kHeaderSize && std::memcmp(data.data(), kMagic, 4) == 0) {
    return selectLegacyBinaryBundle(data, stage, preferredFormat);
  }

  data = trimLeft(data);
  rapidjson::Document doc;
  if (!parseJsonBundle(data, doc)) {
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

    Entry entry;
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
        kEncodingBase64) {
      return std::nullopt;
    }
    if (!base64Decode(StringView(jsonEntry["code"].GetString(),
                                 jsonEntry["code"].GetStringLength()),
                      entry.code)) {
      return std::nullopt;
    }
    return entry;
  }

  return std::nullopt;
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
