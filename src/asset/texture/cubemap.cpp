#include "../../graphics/graphics_system.hpp"
#include "assimp/Logger.hpp"
#include "texture_data.hpp"
#include <asset/texture/cubemap.hpp>
#include <core/io/asset_io.hpp>
#include <core/logger/logger.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <cctype>
#include <cmath>
#include <cstring>
#include <iostream>
#include <optional>
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

static inline void sampleEquirectBilinear(const float *img, int W, int H, int C,
                                          float u, float v,
                                          float *out) // out[C]
{
  u = u - std::floor(u);     // wrap
  v = clampf(v, 0.0f, 1.0f); // clamp

  float x = u * (W - 1);
  float y = v * (H - 1);

  int x0 = (int)std::floor(x);
  int y0 = (int)std::floor(y);
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
                       std::array<std::vector<float>, 6> &outFaces) {
  assert(C == 3 || C == 4);
  for (int f = 0; f < 6; ++f) {
    outFaces[f].assign(faceSize * faceSize * C, 0.0f);
  }

  const float invN = 1.0f / faceSize;
  const float PI = 3.14159265358979323846f;
  const float INV_2PI = 1.0f / (2.0f * PI);
  const float INV_PI = 1.0f / PI;

  std::vector<float> tmp(C, 0.0f);

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

bool loadEXRFloat(const char *path, std::vector<float> &img, int &W, int &H,
                  int &C) {
  float *out;
  const char *err = nullptr;
  int w, h;
  int ret = LoadEXR(&out, &w, &h, path, &err);
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

  std::vector<float> r(W * H), g(W * H), b(W * H), a;
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

  std::vector<float *> img_ptrs;
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

static void writeTexture(px::Ptr<px::Texture> texture,
                         const std::array<std::vector<float>, 6> &faces) {
  auto allocator = GraphicsSystem::getAllocator();
  auto device = GraphicsSystem::getDevice();
  uint32_t width = texture->getCreateInfo().width,
           height = texture->getCreateInfo().height;

  std::vector<float> data;
  for (int i = 0; i < faces.size(); ++i) {
    data.insert(data.end(), faces[i].begin(), faces[i].end());
  }

  px::Ptr<px::TransferBuffer> transbuffers[6];
  for (int i = 0; i < 6; ++i) {
    px::TransferBuffer::CreateInfo info{};
    info.allocator = allocator;
    info.size = width * height * 4 * sizeof(float);
    info.usage = px::TransferBufferUsage::Upload;
    transbuffers[i] = device->CreateTransferBuffer(info);
    auto *pMapped = transbuffers[i]->Map(true);
    memcpy(pMapped, faces[i].data(), info.size);
    transbuffers[i]->Unmap();
  }
  {
    px::CommandBuffer::CreateInfo info{};
    info.allocator = allocator;
    auto commandBuffer = device->AcquireCommandBuffer(info);
    auto copyPass = commandBuffer->BeginCopyPass();

    for (int i = 0; i < 6; i++) {
      px::TextureTransferInfo src{};
      src.offset = 0;
      src.transferBuffer = transbuffers[i];
      px::TextureRegion dst{};
      dst.layer = i;
      dst.x = 0;
      dst.y = 0;
      dst.width = width;
      dst.height = height;
      dst.depth = 1;
      dst.texture = texture;
      copyPass->UploadTexture(src, dst, false);
    }

    commandBuffer->EndCopyPass(copyPass);
    device->SubmitCommandBuffer(commandBuffer);
  }
  device->WaitForGPUIdle();
}
px::Ptr<px::Texture>
createNativeCubemapTexture(const std::array<std::vector<float>, 6> &faces,
                           px::TextureFormat textureFormat, uint32_t width,
                           uint32_t height) {
  auto allocator = GraphicsSystem::getAllocator();
  auto device = GraphicsSystem::getDevice();

  Ptr<px::Texture> texture;
  {
    px::Texture::CreateInfo info{};
    info.allocator = allocator;
    info.width = width;
    info.height = height;
    info.layerCountOrDepth = 6;
    info.format = textureFormat;
    info.usage = px::TextureUsage::Sampler;
    info.numLevels = 1;
    info.sampleCount = px::SampleCount::x1;
    info.type = px::TextureType::Cube;
    texture = device->CreateTexture(info);
  }
  writeTexture(texture, faces);
  return texture;
}
bool Cubemap::load(std::string_view path) {
  auto convertedPath = AssetIO::convertFilePath(AssetType::Texture, path);

  std::vector<float> equirect;
  int W = 0, H = 0, C = 0;
  if (!loadEXRFloat(convertedPath.c_str(), equirect, W, H, C)) {
    return false;
  }

  int faceSize = 1024;

  std::array<std::vector<float>, 6> faces;
  equirectToCubemap(equirect.data(), W, H, C, faceSize, faces);

  this->nativeCubemap = createNativeCubemapTexture(
      faces, px::TextureFormat::R32G32B32A32_FLOAT, faceSize, faceSize);
  return true;
}
} // namespace sinen