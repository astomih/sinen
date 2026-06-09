#include "shader_bundle.hpp"


#include <cstring>
#include <limits>

namespace sinen {
namespace {
constexpr char kMagic[4] = {'S', 'N', 'S', 'B'};
constexpr uint32_t kVersion = 1;
constexpr uint32_t kHeaderSize = 12;
constexpr uint32_t kEntrySize = 36;

void appendU32(Array<char> &out, uint32_t value) {
  out.push_back(static_cast<char>(value & 0xffu));
  out.push_back(static_cast<char>((value >> 8u) & 0xffu));
  out.push_back(static_cast<char>((value >> 16u) & 0xffu));
  out.push_back(static_cast<char>((value >> 24u) & 0xffu));
}

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

void align4(Array<char> &out) {
  while ((out.size() % 4) != 0) {
    out.push_back(0);
  }
}

uint32_t checkedU32(size_t value) {
  if (value > std::numeric_limits<uint32_t>::max()) {
    return 0;
  }
  return static_cast<uint32_t>(value);
}

} // namespace

bool ShaderBundle::isBundle(StringView data) {
  return data.size() >= kHeaderSize && std::memcmp(data.data(), kMagic, 4) == 0;
}

Array<char> ShaderBundle::pack(const Array<PackEntry> &entries) {
  Array<char> out;
  const size_t tableSize = static_cast<size_t>(kEntrySize) * entries.size();
  const size_t payloadOffset = kHeaderSize + tableSize;
  if (payloadOffset > std::numeric_limits<uint32_t>::max()) {
    return out;
  }

  Array<uint32_t> offsets;
  Array<uint32_t> sizes;
  offsets.reserve(entries.size());
  sizes.reserve(entries.size());

  Array<char> payload;
  for (const auto &entry : entries) {
    align4(payload);
    offsets.push_back(checkedU32(payloadOffset + payload.size()));
    sizes.push_back(entry.size);
    appendBytes(payload, entry.data, entry.size);
  }

  appendBytes(out, kMagic, 4);
  appendU32(out, kVersion);
  appendU32(out, checkedU32(entries.size()));

  for (size_t i = 0; i < entries.size(); ++i) {
    const auto &entry = entries[i];
    appendU32(out, static_cast<uint32_t>(entry.stage));
    appendU32(out, static_cast<uint32_t>(entry.format));
    appendU32(out, offsets[i]);
    appendU32(out, sizes[i]);
    appendU32(out, entry.numSamplers);
    appendU32(out, entry.numStorageBuffers);
    appendU32(out, entry.numStorageTextures);
    appendU32(out, entry.numUniformBuffers);
    appendU32(out, 0);
  }

  out.insert(out.end(), payload.begin(), payload.end());
  return out;
}

Buffer ShaderBundle::packBuffer(const Array<PackEntry> &entries) {
  auto bundle = pack(entries);
  Buffer buffer = makeBuffer(bundle.size(), BufferType::Binary);
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
    Entry entry;
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
