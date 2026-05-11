#include <core/thread/global_thread_pool.hpp>
#include <core/thread/load_context.hpp>
#include <graphics/graphics.hpp>
#include <graphics/texture/texture.hpp>
#include <math/math.hpp>
#include <platform/io/asset_io.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstring>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <string>

#define TINYEXR_IMPLEMENTATION
#include <tinyexr.h>
#include <vector>
namespace sinen {

static inline float clampf(float x, float a, float b) {
  return x < a ? a : (x > b ? b : x);
}
static inline void normalize3(float &x, float &y, float &z) {
  float len = std::sqrt(x * x + y * y + z * z);
  if (len > 0.0f) {
    x /= len;
    y /= len;
    z /= len;
  }
}

struct Float2 {
  float x = 0.0f;
  float y = 0.0f;
};

struct Float3 {
  float x = 0.0f;
  float y = 0.0f;
  float z = 0.0f;
};

static inline Float3 makeFloat3(float x, float y, float z) {
  return Float3{x, y, z};
}

static inline Float3 add(Float3 a, Float3 b) {
  return makeFloat3(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline Float3 sub(Float3 a, Float3 b) {
  return makeFloat3(a.x - b.x, a.y - b.y, a.z - b.z);
}

static inline Float3 mul(Float3 v, float s) {
  return makeFloat3(v.x * s, v.y * s, v.z * s);
}

static inline float dot(Float3 a, Float3 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline Float3 cross(Float3 a, Float3 b) {
  return makeFloat3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
                    a.x * b.y - a.y * b.x);
}

static inline Float3 normalize(Float3 v) {
  float len = std::sqrt(dot(v, v));
  if (len <= 0.0f) {
    return makeFloat3(0.0f, 0.0f, 0.0f);
  }
  return mul(v, 1.0f / len);
}

static inline Float2 hammersley(uint32_t i, uint32_t n) {
  uint32_t bits = i;
  bits = (bits << 16u) | (bits >> 16u);
  bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
  bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
  bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
  bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
  const float radicalInverse =
      static_cast<float>(bits) * 2.3283064365386963e-10f;
  return Float2{static_cast<float>(i) / static_cast<float>(n), radicalInverse};
}

static inline void tangentBasis(Float3 n, Float3 &tangent, Float3 &bitangent) {
  const Float3 up = std::fabs(n.z) < 0.999f ? makeFloat3(0.0f, 0.0f, 1.0f)
                                            : makeFloat3(1.0f, 0.0f, 0.0f);
  tangent = normalize(cross(up, n));
  bitangent = cross(n, tangent);
}

static inline void sampleEquirectBilinear(const float *img, int W, int H, int C,
                                          float u, float v,
                                          float *out) // out[C]
{
  u = u - Math::floor(u);    // wrap
  v = clampf(v, 0.0f, 1.0f); // clamp

  float x = u * (W - 1);
  float y = v * (H - 1);

  int x0 = (int)Math::floor(x);
  int y0 = (int)Math::floor(y);
  int x1 = (x0 + 1) % W;            // wrap horizontally
  int y1 = std::min(y0 + 1, H - 1); // clamp vertically

  float tx = x - x0;
  float ty = y - y0;

  const float *p00 = img + (y0 * W + x0) * C;
  const float *p10 = img + (y0 * W + x1) * C;
  const float *p01 = img + (y1 * W + x0) * C;
  const float *p11 = img + (y1 * W + x1) * C;

  for (int c = 0; c < C; ++c) {
    float v0 = p00[c] * (1.0f - tx) + p10[c] * tx;
    float v1 = p01[c] * (1.0f - tx) + p11[c] * tx;
    out[c] = v0 * (1.0f - ty) + v1 * ty;
  }
}

static inline Float3 sampleEquirectDirection(const float *img, int W, int H,
                                             int C, Float3 dir) {
  constexpr float PI = 3.14159265358979323846f;
  constexpr float INV_2PI = 1.0f / (2.0f * PI);
  constexpr float INV_PI = 1.0f / PI;
  float rgba[4] = {};
  const float u = std::atan2(dir.z, dir.x) * INV_2PI + 0.5f;
  const float v = std::acos(clampf(dir.y, -1.0f, 1.0f)) * INV_PI;
  sampleEquirectBilinear(img, W, H, C, u, v, rgba);
  return makeFloat3(rgba[0], rgba[1], rgba[2]);
}

// 0:+X, 1:-X, 2:+Y, 3:-Y, 4:+Z, 5:-Z
enum Face { PX = 0, NX = 1, PY = 2, NY = 3, PZ = 4, NZ = 5 };

static inline void faceDir(Face f, float a, float b, float &x, float &y,
                           float &z) {
  switch (f) {
  case PX:
    x = 1.0f;
    y = b;
    z = -a;
    break;
  case NX:
    x = -1.0f;
    y = b;
    z = a;
    break;
  case PY:
    x = a;
    y = 1.0f;
    z = -b;
    break;
  case NY:
    x = a;
    y = -1.0f;
    z = b;
    break;
  case PZ:
    x = a;
    y = b;
    z = 1.0f;
    break;
  case NZ:
    x = -a;
    y = b;
    z = -1.0f;
    break;
  }
  normalize3(x, y, z);
}

void equirectToCubemap(const float *in, int W, int H, int C, int faceSize,
                       std::array<Array<float>, 6> &outFaces) {
  assert(C == 3 || C == 4);
  for (int f = 0; f < 6; ++f) {
    outFaces[f].assign(faceSize * faceSize * C, 0.0f);
  }

  const float invN = 1.0f / faceSize;
  const float PI = 3.14159265358979323846f;
  const float INV_2PI = 1.0f / (2.0f * PI);
  const float INV_PI = 1.0f / PI;

  Array<float> tmp(C, 0.0f);

  for (int f = 0; f < 6; ++f) {
    float *dst = outFaces[f].data();
    for (int j = 0; j < faceSize; ++j) {
      float v = (j + 0.5f) * invN; // [0,1]
      float b = -(2.0f * v - 1.0f);

      for (int i = 0; i < faceSize; ++i) {
        float u = (i + 0.5f) * invN; // [0,1]
        float a = 2.0f * u - 1.0f;   // [-1,1]

        float rx, ry, rz;
        faceDir(static_cast<Face>(f), a, b, rx, ry, rz);

        float U = std::atan2(rz, rx) * INV_2PI + 0.5f;
        float V = std::acos(clampf(ry, -1.0f, 1.0f)) * INV_PI;

        sampleEquirectBilinear(in, W, H, C, U, V, tmp.data());

        float *px = dst + (j * faceSize + i) * C;
        for (int c = 0; c < C; ++c)
          px[c] = tmp[c];
      }
    }
  }
}

void computeIrradianceCubemap(const float *in, int W, int H, int C,
                              int faceSize, uint32_t sampleCount,
                              std::array<Array<float>, 6> &outFaces) {
  assert(C == 3 || C == 4);
  sampleCount = Math::max(1u, sampleCount);
  for (int f = 0; f < 6; ++f) {
    outFaces[f].assign(faceSize * faceSize * 4, 0.0f);
  }

  constexpr float PI = 3.14159265358979323846f;
  const float invN = 1.0f / faceSize;

  for (int f = 0; f < 6; ++f) {
    float *dst = outFaces[f].data();
    for (int j = 0; j < faceSize; ++j) {
      const float v = (j + 0.5f) * invN;
      const float b = -(2.0f * v - 1.0f);

      for (int i = 0; i < faceSize; ++i) {
        const float u = (i + 0.5f) * invN;
        const float a = 2.0f * u - 1.0f;

        float nx, ny, nz;
        faceDir(static_cast<Face>(f), a, b, nx, ny, nz);
        const Float3 n = makeFloat3(nx, ny, nz);

        Float3 tangent;
        Float3 bitangent;
        tangentBasis(n, tangent, bitangent);

        Float3 irradiance = {};
        for (uint32_t sample = 0; sample < sampleCount; ++sample) {
          const Float2 xi = hammersley(sample, sampleCount);
          const float phi = 2.0f * PI * xi.x;
          const float cosTheta = std::sqrt(1.0f - xi.y);
          const float sinTheta = std::sqrt(xi.y);

          const Float3 local = makeFloat3(std::cos(phi) * sinTheta,
                                          std::sin(phi) * sinTheta, cosTheta);
          const Float3 dir =
              normalize(add(add(mul(tangent, local.x), mul(bitangent, local.y)),
                            mul(n, local.z)));
          irradiance =
              add(irradiance, sampleEquirectDirection(in, W, H, C, dir));
        }

        // Match the engine's existing irradiance maps: store the
        // cosine-weighted average radiance. The sample shader applies its own
        // diffuse scale.
        irradiance = mul(irradiance, 1.0f / static_cast<float>(sampleCount));
        float *px = dst + (j * faceSize + i) * 4;
        px[0] = irradiance.x;
        px[1] = irradiance.y;
        px[2] = irradiance.z;
        px[3] = 1.0f;
      }
    }
  }
}

struct CubeMipFaces {
  std::array<Array<float>, 6> faces;
  uint32_t faceSize = 0;
};

static inline Float3 importanceSampleGGX(Float2 xi, Float3 n, float roughness) {
  constexpr float PI = 3.14159265358979323846f;
  const float a = roughness * roughness;
  const float phi = 2.0f * PI * xi.x;
  const float cosTheta =
      std::sqrt((1.0f - xi.y) / (1.0f + (a * a - 1.0f) * xi.y));
  const float sinTheta = std::sqrt(Math::max(0.0f, 1.0f - cosTheta * cosTheta));

  const Float3 h =
      makeFloat3(std::cos(phi) * sinTheta, std::sin(phi) * sinTheta, cosTheta);

  Float3 tangent;
  Float3 bitangent;
  tangentBasis(n, tangent, bitangent);
  return normalize(
      add(add(mul(tangent, h.x), mul(bitangent, h.y)), mul(n, h.z)));
}

static inline float geometrySchlickGGX(float nDotV, float roughness) {
  const float k = (roughness * roughness) * 0.5f;
  return nDotV / (nDotV * (1.0f - k) + k);
}

static inline float geometrySmith(Float3 n, Float3 v, Float3 l,
                                  float roughness) {
  const float nDotV = Math::max(dot(n, v), 0.0f);
  const float nDotL = Math::max(dot(n, l), 0.0f);
  return geometrySchlickGGX(nDotV, roughness) *
         geometrySchlickGGX(nDotL, roughness);
}

static uint32_t maxMipLevels(uint32_t faceSize) {
  uint32_t levels = 0;
  do {
    ++levels;
    faceSize >>= 1u;
  } while (faceSize > 0);
  return levels;
}

void computePrefilteredCubemap(const float *in, int W, int H, int C,
                               uint32_t faceSize, uint32_t mipLevels,
                               uint32_t sampleCount,
                               Array<CubeMipFaces> &outMips) {
  assert(C == 3 || C == 4);
  faceSize = Math::max(1u, faceSize);
  mipLevels = Math::max(1u, Math::min(mipLevels, maxMipLevels(faceSize)));
  sampleCount = Math::max(1u, sampleCount);

  outMips.clear();
  outMips.resize(mipLevels);

  for (uint32_t mip = 0; mip < mipLevels; ++mip) {
    const uint32_t mipFaceSize = Math::max(1u, faceSize >> mip);
    const float roughness =
        mipLevels > 1 ? static_cast<float>(mip) / (mipLevels - 1u) : 0.0f;

    outMips[mip].faceSize = mipFaceSize;
    for (int f = 0; f < 6; ++f) {
      outMips[mip].faces[f].assign(mipFaceSize * mipFaceSize * 4, 0.0f);
    }

    const float invN = 1.0f / mipFaceSize;
    for (int f = 0; f < 6; ++f) {
      float *dst = outMips[mip].faces[f].data();
      for (uint32_t j = 0; j < mipFaceSize; ++j) {
        const float v = (j + 0.5f) * invN;
        const float b = -(2.0f * v - 1.0f);

        for (uint32_t i = 0; i < mipFaceSize; ++i) {
          const float u = (i + 0.5f) * invN;
          const float a = 2.0f * u - 1.0f;

          float nx, ny, nz;
          faceDir(static_cast<Face>(f), a, b, nx, ny, nz);
          const Float3 n = makeFloat3(nx, ny, nz);
          const Float3 vdir = n;

          Float3 prefiltered = {};
          float totalWeight = 0.0f;
          for (uint32_t sample = 0; sample < sampleCount; ++sample) {
            const Float2 xi = hammersley(sample, sampleCount);
            const Float3 h = importanceSampleGGX(xi, n, roughness);
            const Float3 l = normalize(sub(mul(h, 2.0f * dot(vdir, h)), vdir));
            const float nDotL = Math::max(dot(n, l), 0.0f);
            if (nDotL <= 0.0f) {
              continue;
            }

            prefiltered =
                add(prefiltered,
                    mul(sampleEquirectDirection(in, W, H, C, l), nDotL));
            totalWeight += nDotL;
          }

          if (totalWeight > 0.0f) {
            prefiltered = mul(prefiltered, 1.0f / totalWeight);
          } else {
            prefiltered = sampleEquirectDirection(in, W, H, C, n);
          }

          float *px = dst + (j * mipFaceSize + i) * 4;
          px[0] = prefiltered.x;
          px[1] = prefiltered.y;
          px[2] = prefiltered.z;
          px[3] = 1.0f;
        }
      }
    }
  }
}

void computeBRDFLUT(uint32_t size, uint32_t sampleCount, Array<float> &out) {
  size = Math::max(1u, size);
  sampleCount = Math::max(1u, sampleCount);
  out.assign(static_cast<size_t>(size) * size * 4u, 0.0f);

  const Float3 n = makeFloat3(0.0f, 0.0f, 1.0f);
  for (uint32_t y = 0; y < size; ++y) {
    const float roughness = (static_cast<float>(y) + 0.5f) / size;
    for (uint32_t x = 0; x < size; ++x) {
      const float nDotV = (static_cast<float>(x) + 0.5f) / size;
      const Float3 v = makeFloat3(
          std::sqrt(Math::max(0.0f, 1.0f - nDotV * nDotV)), 0.0f, nDotV);

      float a = 0.0f;
      float b = 0.0f;
      for (uint32_t i = 0; i < sampleCount; ++i) {
        const Float2 xi = hammersley(i, sampleCount);
        const Float3 h = importanceSampleGGX(xi, n, roughness);
        const Float3 l = normalize(sub(mul(h, 2.0f * dot(v, h)), v));

        const float nDotL = Math::max(l.z, 0.0f);
        const float nDotH = Math::max(h.z, 0.0f);
        const float vDotH = Math::max(dot(v, h), 0.0f);
        if (nDotL > 0.0f && nDotH > 0.0f && nDotV > 0.0f) {
          const float g = geometrySmith(n, v, l, roughness);
          const float gVis = (g * vDotH) / (nDotH * nDotV);
          const float fc = std::pow(1.0f - vDotH, 5.0f);
          a += (1.0f - fc) * gVis;
          b += fc * gVis;
        }
      }

      // Match the existing BRDF LUT texture orientation used by shader UVs.
      const uint32_t dstY = size - 1u - y;
      const size_t index = (static_cast<size_t>(dstY) * size + x) * 4u;
      out[index + 0] = a / static_cast<float>(sampleCount);
      out[index + 1] = b / static_cast<float>(sampleCount);
      out[index + 2] = 0.0f;
      out[index + 3] = 1.0f;
    }
  }
}

bool loadEXRFloat(StringView path, Array<float> &img, int &W, int &H, int &C) {
  float *out;
  const char *err = nullptr;
  int w, h;
  int ret = TINYEXR_ERROR_INVALID_ARGUMENT;
  String bytes;
  if (AssetIO::isArchiveMounted() && AssetIO::exists(path)) {
    bytes = AssetIO::openAsString(path);
    ret = LoadEXRFromMemory(
        &out, &w, &h, reinterpret_cast<const unsigned char *>(bytes.data()),
        bytes.size(), &err);
  } else {
    auto filePath = AssetIO::getFilePath(path);
    ret = LoadEXR(&out, &w, &h, filePath.c_str(), &err);
  }
  if (ret != TINYEXR_SUCCESS) {
    if (err) {
      std::cerr << "LoadEXR error: " << err << "\n";
      FreeEXRErrorMessage(err);
    }
    return false;
  }
  W = w;
  H = h;
  C = 4;
  img.assign(out, out + size_t(W) * H * C);
  free(out);
  return true;
}

bool saveEXRFloat(const char *path, const float *img, int W, int H, int C) {
  EXRHeader header;
  InitEXRHeader(&header);
  EXRImage image;
  InitEXRImage(&image);

  image.num_channels = C;

  Array<float> r(W * H), g(W * H), b(W * H), a;
  std::array<float *, 4> channels{};
  for (int i = 0; i < W * H; ++i) {
    r[i] = img[i * C + 0];
    g[i] = img[i * C + 1];
    b[i] = img[i * C + 2];
  }
  if (C == 4) {
    a.resize(W * H);
    for (int i = 0; i < W * H; ++i)
      a[i] = img[i * C + 3];
  }

  Array<float *> img_ptrs;
  img_ptrs.push_back(b.data());
  img_ptrs.push_back(g.data());
  img_ptrs.push_back(r.data());
  if (C == 4)
    img_ptrs.push_back(a.data());

  image.images = reinterpret_cast<unsigned char **>(img_ptrs.data());
  image.width = W;
  image.height = H;

  header.num_channels = C;
  header.channels =
      (EXRChannelInfo *)malloc(sizeof(EXRChannelInfo) * header.num_channels);

  const char *names3[3] = {"B", "G", "R"};
  const char *names4[4] = {"B", "G", "R", "A"};
  for (int i = 0; i < C; ++i) {
    const char *nm = (C == 3 ? names3[i] : names4[i]);
    strncpy(header.channels[i].name, nm, 255);
    header.channels[i].name[255] = '\0';
  }

  header.pixel_types = (int *)malloc(sizeof(int) * C);
  header.requested_pixel_types = (int *)malloc(sizeof(int) * C);
  for (int i = 0; i < C; i++) {
    header.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;
    header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;
  }

  const char *err = nullptr;
  int ret = SaveEXRImageToFile(&image, &header, path, &err);
  if (ret != TINYEXR_SUCCESS) {
    if (err) {
      std::cerr << "SaveEXR error: " << err << "\n";
      FreeEXRErrorMessage(err);
    }
    free(header.channels);
    free(header.pixel_types);
    free(header.requested_pixel_types);
    return false;
  }

  free(header.channels);
  free(header.pixel_types);
  free(header.requested_pixel_types);
  return true;
}

static void writeTexture(Ptr<gpu::Texture> texture,
                         const std::array<Array<float>, 6> &faces) {
  auto device = Graphics::getDevice();
  uint32_t width = texture->getCreateInfo().width,
           height = texture->getCreateInfo().height;

  Array<float> data;
  for (int i = 0; i < faces.size(); ++i) {
    data.insert(data.end(), faces[i].begin(), faces[i].end());
  }

  Ptr<gpu::TransferBuffer> transbuffers[6];
  for (int i = 0; i < 6; ++i) {
    gpu::TransferBuffer::CreateInfo info{};
    info.allocator = GlobalAllocator::get();
    info.size = width * height * 4 * sizeof(Float32);
    info.usage = gpu::TransferBufferUsage::Upload;
    transbuffers[i] = device->createTransferBuffer(info);
    auto *pMapped = transbuffers[i]->map(true);
    memcpy(pMapped, faces[i].data(), info.size);
    transbuffers[i]->unmap();
  }
  {
    gpu::CommandBuffer::CreateInfo info{};
    info.allocator = GlobalAllocator::get();
    auto commandBuffer = device->acquireCommandBuffer(info);
    auto copyPass = commandBuffer->beginCopyPass();

    for (int i = 0; i < 6; i++) {
      gpu::TextureTransferInfo src{};
      src.offset = 0;
      src.transferBuffer = transbuffers[i];
      gpu::TextureRegion dst{};
      dst.layer = i;
      dst.x = 0;
      dst.y = 0;
      dst.width = width;
      dst.height = height;
      dst.depth = 1;
      dst.texture = texture;
      copyPass->uploadTexture(src, dst, false);
    }

    commandBuffer->endCopyPass(copyPass);
    device->submitCommandBuffer(commandBuffer);
  }
  device->waitForGpuIdle();
}

static void writeTexture(Ptr<gpu::Texture> texture,
                         const Array<CubeMipFaces> &mips) {
  auto device = Graphics::getDevice();

  for (uint32_t mip = 0; mip < mips.size(); ++mip) {
    const auto &mipData = mips[mip];
    Ptr<gpu::TransferBuffer> transbuffers[6];
    for (int face = 0; face < 6; ++face) {
      gpu::TransferBuffer::CreateInfo info{};
      info.allocator = GlobalAllocator::get();
      info.size = mipData.faceSize * mipData.faceSize * 4 * sizeof(Float32);
      info.usage = gpu::TransferBufferUsage::Upload;
      transbuffers[face] = device->createTransferBuffer(info);
      auto *pMapped = transbuffers[face]->map(true);
      memcpy(pMapped, mipData.faces[face].data(), info.size);
      transbuffers[face]->unmap();
    }

    gpu::CommandBuffer::CreateInfo info{};
    info.allocator = GlobalAllocator::get();
    auto commandBuffer = device->acquireCommandBuffer(info);
    auto copyPass = commandBuffer->beginCopyPass();

    for (int face = 0; face < 6; ++face) {
      gpu::TextureTransferInfo src{};
      src.offset = 0;
      src.transferBuffer = transbuffers[face];
      gpu::TextureRegion dst{};
      dst.layer = face;
      dst.mipLevel = mip;
      dst.x = 0;
      dst.y = 0;
      dst.width = mipData.faceSize;
      dst.height = mipData.faceSize;
      dst.depth = 1;
      dst.texture = texture;
      copyPass->uploadTexture(src, dst, false);
    }

    commandBuffer->endCopyPass(copyPass);
    device->submitCommandBuffer(commandBuffer);
  }
  device->waitForGpuIdle();
}

static void writeFloatTexture2D(Ptr<gpu::Texture> texture,
                                const Array<float> &pixels) {
  auto device = Graphics::getDevice();
  const uint32_t width = texture->getCreateInfo().width;
  const uint32_t height = texture->getCreateInfo().height;

  Ptr<gpu::TransferBuffer> transferBuffer;
  {
    gpu::TransferBuffer::CreateInfo info{};
    info.allocator = GlobalAllocator::get();
    info.size = width * height * 4 * sizeof(Float32);
    info.usage = gpu::TransferBufferUsage::Upload;
    transferBuffer = device->createTransferBuffer(info);
    auto *pMapped = transferBuffer->map(true);
    memcpy(pMapped, pixels.data(), info.size);
    transferBuffer->unmap();
  }
  {
    gpu::CommandBuffer::CreateInfo info{};
    info.allocator = GlobalAllocator::get();
    auto commandBuffer = device->acquireCommandBuffer(info);
    auto copyPass = commandBuffer->beginCopyPass();

    gpu::TextureTransferInfo src{};
    src.offset = 0;
    src.transferBuffer = transferBuffer;
    gpu::TextureRegion dst{};
    dst.x = 0;
    dst.y = 0;
    dst.width = width;
    dst.height = height;
    dst.depth = 1;
    dst.texture = texture;
    copyPass->uploadTexture(src, dst, true);

    commandBuffer->endCopyPass(copyPass);
    device->submitCommandBuffer(commandBuffer);
  }
  device->waitForGpuIdle();
}

Ptr<gpu::Texture>
createNativeCubemapTexture(const std::array<Array<float>, 6> &faces,
                           gpu::TextureFormat textureFormat, uint32_t width,
                           uint32_t height) {
  auto device = Graphics::getDevice();

  Ptr<gpu::Texture> texture;
  {
    gpu::Texture::CreateInfo info{};
    info.allocator = GlobalAllocator::get();
    info.width = width;
    info.height = height;
    info.layerCountOrDepth = 6;
    info.format = textureFormat;
    info.usage = gpu::TextureUsage::Sampler;
    info.numLevels = 1;
    info.sampleCount = gpu::SampleCount::x1;
    info.type = gpu::TextureType::Cube;
    texture = device->createTexture(info);
  }
  writeTexture(texture, faces);
  return texture;
}

Ptr<gpu::Texture> createNativeCubemapTexture(const Array<CubeMipFaces> &mips,
                                             gpu::TextureFormat textureFormat) {
  if (mips.empty()) {
    return nullptr;
  }

  auto device = Graphics::getDevice();

  gpu::Texture::CreateInfo info{};
  info.allocator = GlobalAllocator::get();
  info.width = mips[0].faceSize;
  info.height = mips[0].faceSize;
  info.layerCountOrDepth = 6;
  info.format = textureFormat;
  info.usage = gpu::TextureUsage::Sampler;
  info.numLevels = static_cast<UInt32>(mips.size());
  info.sampleCount = gpu::SampleCount::x1;
  info.type = gpu::TextureType::Cube;
  auto texture = device->createTexture(info);
  writeTexture(texture, mips);
  return texture;
}

Ptr<gpu::Texture> createNativeFloatTexture2D(const Array<float> &pixels,
                                             uint32_t width, uint32_t height) {
  auto device = Graphics::getDevice();

  gpu::Texture::CreateInfo info{};
  info.allocator = GlobalAllocator::get();
  info.width = width;
  info.height = height;
  info.layerCountOrDepth = 1;
  info.format = gpu::TextureFormat::R32G32B32A32_FLOAT;
  info.usage = gpu::TextureUsage::Sampler;
  info.numLevels = 1;
  info.sampleCount = gpu::SampleCount::x1;
  info.type = gpu::TextureType::Texture2D;
  auto texture = device->createTexture(info);
  writeFloatTexture2D(texture, pixels);
  return texture;
}

bool Texture::loadCubemap(StringView path) {
  this->loading = true;
  this->pendingWidth = 0;
  this->pendingHeight = 0;

  const TaskGroup group = LoadContext::current();
  group.add();

  struct AsyncCubemapState {
    std::future<void> future;
    std::array<Array<float>, 6> faces;
    uint32_t faceSize = 0;
    bool ok = false;
  };

  auto state = makePtr<AsyncCubemapState>();
  this->async = state;

  const String srcPath = path.data();
  state->future = globalThreadPool().submit([state, srcPath] {
    Array<float> equirect;
    int w = 0, h = 0, c = 0;
    if (!loadEXRFloat(srcPath, equirect, w, h, c)) {
      state->ok = false;
      return;
    }

    const uint32_t faceSize = 1024;
    std::array<Array<float>, 6> faces;
    equirectToCubemap(equirect.data(), w, h, c, faceSize, faces);

    state->faces = std::move(faces);
    state->faceSize = faceSize;
    state->ok = true;
  });

  auto pollAndUpload = std::make_shared<std::function<void()>>();
  *pollAndUpload = [this, pollAndUpload, state, group]() {
    if (!state->future.valid()) {
      this->loading = false;
      this->async.reset();
      group.done();
      return;
    }

    if (state->future.wait_for(std::chrono::milliseconds(0)) !=
        std::future_status::ready) {
      Graphics::addPreDrawFunc(*pollAndUpload);
      return;
    }

    state->future.get();
    if (state->ok) {
      texture = createNativeCubemapTexture(
          state->faces, gpu::TextureFormat::R32G32B32A32_FLOAT, state->faceSize,
          state->faceSize);
      this->pendingWidth = state->faceSize;
      this->pendingHeight = state->faceSize;
    }
    this->loading = false;
    this->async.reset();
    assert(this->texture);
    group.done();
  };
  Graphics::addPreDrawFunc(*pollAndUpload);
  return true;
}

bool Texture::loadIrradianceCubemap(StringView path, uint32_t faceSize,
                                    uint32_t sampleCount) {
  if (faceSize == 0 || sampleCount == 0) {
    return false;
  }

  this->loading = true;
  this->pendingWidth = 0;
  this->pendingHeight = 0;

  const TaskGroup group = LoadContext::current();
  group.add();

  struct AsyncIrradianceState {
    std::future<void> future;
    std::array<Array<float>, 6> faces;
    uint32_t faceSize = 0;
    bool ok = false;
  };

  auto state = makePtr<AsyncIrradianceState>();
  this->async = state;

  const String srcPath = path.data();
  state->future = globalThreadPool().submit([state, srcPath, faceSize,
                                             sampleCount] {
    Array<float> equirect;
    int w = 0, h = 0, c = 0;
    if (!loadEXRFloat(srcPath, equirect, w, h, c)) {
      state->ok = false;
      return;
    }

    std::array<Array<float>, 6> faces;
    computeIrradianceCubemap(equirect.data(), w, h, c,
                             static_cast<int>(faceSize), sampleCount, faces);

    state->faces = std::move(faces);
    state->faceSize = faceSize;
    state->ok = true;
  });

  auto pollAndUpload = std::make_shared<std::function<void()>>();
  *pollAndUpload = [this, pollAndUpload, state, group]() {
    if (!state->future.valid()) {
      this->loading = false;
      this->async.reset();
      group.done();
      return;
    }

    if (state->future.wait_for(std::chrono::milliseconds(0)) !=
        std::future_status::ready) {
      Graphics::addPreDrawFunc(*pollAndUpload);
      return;
    }

    state->future.get();
    if (state->ok) {
      texture = createNativeCubemapTexture(
          state->faces, gpu::TextureFormat::R32G32B32A32_FLOAT, state->faceSize,
          state->faceSize);
      this->pendingWidth = state->faceSize;
      this->pendingHeight = state->faceSize;
    }
    this->loading = false;
    this->async.reset();
    group.done();
  };
  Graphics::addPreDrawFunc(*pollAndUpload);
  return true;
}

bool Texture::loadPrefilteredCubemap(StringView path, uint32_t faceSize,
                                     uint32_t mipLevels, uint32_t sampleCount) {
  if (faceSize == 0 || mipLevels == 0 || sampleCount == 0) {
    return false;
  }

  this->loading = true;
  this->pendingWidth = 0;
  this->pendingHeight = 0;

  const TaskGroup group = LoadContext::current();
  group.add();

  struct AsyncPrefilteredState {
    std::future<void> future;
    Array<CubeMipFaces> mips;
    uint32_t faceSize = 0;
    bool ok = false;
  };

  auto state = makePtr<AsyncPrefilteredState>();
  this->async = state;

  const String srcPath = path.data();
  state->future = globalThreadPool().submit(
      [state, srcPath, faceSize, mipLevels, sampleCount] {
        Array<float> equirect;
        int w = 0, h = 0, c = 0;
        if (!loadEXRFloat(srcPath, equirect, w, h, c)) {
          state->ok = false;
          return;
        }

        Array<CubeMipFaces> mips;
        computePrefilteredCubemap(equirect.data(), w, h, c, faceSize, mipLevels,
                                  sampleCount, mips);

        state->mips = std::move(mips);
        state->faceSize = faceSize;
        state->ok = true;
      });

  auto pollAndUpload = std::make_shared<std::function<void()>>();
  *pollAndUpload = [this, pollAndUpload, state, group]() {
    if (!state->future.valid()) {
      this->loading = false;
      this->async.reset();
      group.done();
      return;
    }

    if (state->future.wait_for(std::chrono::milliseconds(0)) !=
        std::future_status::ready) {
      Graphics::addPreDrawFunc(*pollAndUpload);
      return;
    }

    state->future.get();
    if (state->ok) {
      texture = createNativeCubemapTexture(
          state->mips, gpu::TextureFormat::R32G32B32A32_FLOAT);
      this->pendingWidth = state->faceSize;
      this->pendingHeight = state->faceSize;
    }
    this->loading = false;
    this->async.reset();
    group.done();
  };
  Graphics::addPreDrawFunc(*pollAndUpload);
  return true;
}

bool Texture::loadBRDFLUT(uint32_t size, uint32_t sampleCount) {
  if (size == 0 || sampleCount == 0) {
    return false;
  }

  this->loading = true;
  this->pendingWidth = 0;
  this->pendingHeight = 0;

  const TaskGroup group = LoadContext::current();
  group.add();

  struct AsyncBRDFLUTState {
    std::future<void> future;
    Array<float> pixels;
    uint32_t size = 0;
    bool ok = false;
  };

  auto state = makePtr<AsyncBRDFLUTState>();
  this->async = state;

  state->future = globalThreadPool().submit([state, size, sampleCount] {
    computeBRDFLUT(size, sampleCount, state->pixels);
    state->size = size;
    state->ok = true;
  });

  auto pollAndUpload = std::make_shared<std::function<void()>>();
  *pollAndUpload = [this, pollAndUpload, state, group]() {
    if (!state->future.valid()) {
      this->loading = false;
      this->async.reset();
      group.done();
      return;
    }

    if (state->future.wait_for(std::chrono::milliseconds(0)) !=
        std::future_status::ready) {
      Graphics::addPreDrawFunc(*pollAndUpload);
      return;
    }

    state->future.get();
    if (state->ok) {
      texture =
          createNativeFloatTexture2D(state->pixels, state->size, state->size);
      this->pendingWidth = state->size;
      this->pendingHeight = state->size;
    }
    this->loading = false;
    this->async.reset();
    group.done();
  };
  Graphics::addPreDrawFunc(*pollAndUpload);
  return true;
}
} // namespace sinen
