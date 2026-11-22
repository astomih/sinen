#ifndef SINEN_GPU_COPY_PASS_HPP
#define SINEN_GPU_COPY_PASS_HPP
#include "gpu_buffer.hpp"
#include "gpu_texture.hpp"
#include <core/data/ptr.hpp>
#include <core/def/types.hpp>

namespace sinen::gpu {
struct TextureRegion {
  Ptr<class Texture> texture;
  UInt32 mipLevel;
  UInt32 layer;
  UInt32 x, y, z;
  UInt32 width;
  UInt32 height;
  UInt32 depth;
};
struct TextureTransferInfo {
  Ptr<class TransferBuffer> transferBuffer;
  UInt32 offset;
};
struct TextureLocation {
  Ptr<Texture> texture;
  UInt32 mipLevel;
  UInt32 layer;
  UInt32 x;
  UInt32 y;
  UInt32 z;
};
struct BufferTransferInfo {
  Ptr<class TransferBuffer> transferBuffer;
  UInt32 offset;
};
class CopyPass {
public:
  virtual ~CopyPass() = default;

  virtual void uploadTexture(const TextureTransferInfo &src,
                             const TextureRegion &dst, bool cycle) = 0;
  virtual void downloadTexture(const TextureRegion &src,
                               const TextureTransferInfo &dst) = 0;
  virtual void uploadBuffer(const BufferTransferInfo &src,
                            const BufferRegion &dst, bool cycle) = 0;
  virtual void downloadBuffer(const BufferRegion &src,
                              const BufferTransferInfo &dst) = 0;

  // GPU-only, full 2D mip copy for RGBA8/BGRA8/RGBA32_FLOAT. Rows must be
  // padded to 256 bytes; source offset must be a multiple of 512. False means
  // invalid arguments or an unsupported backend.
  virtual bool copyBufferToTexture(const BufferRegion &src,
                                   const TextureRegion &dst,
                                   UInt32 bytesPerRow) {
    return false;
  }

  virtual void copyTexture(const TextureLocation &src,
                           const TextureLocation &dst, UInt32 width,
                           UInt32 height, UInt32 depth, bool cycle) = 0;

protected:
  static bool validBufferTextureCopy(const BufferRegion &src,
                                     const TextureRegion &dst, UInt32 row) {
    if (!src.buffer || !dst.texture || dst.x || dst.y || dst.z || dst.layer ||
        dst.depth != 1 || src.offset % 512 || row % 256)
      return false;
    const auto &info = dst.texture->getCreateInfo();
    if (info.type != TextureType::Texture2D || dst.mipLevel >= info.numLevels ||
        dst.mipLevel >= 32)
      return false;
    UInt32 bpp = 4;
    if (info.format == TextureFormat::R32G32B32A32_FLOAT)
      bpp = 16;
    else if (info.format != TextureFormat::R8G8B8A8_UNORM &&
             info.format != TextureFormat::B8G8R8A8_UNORM)
      return false;
    const UInt32 width = info.width >> dst.mipLevel;
    const UInt32 height = info.height >> dst.mipLevel;
    if (!dst.width || !dst.height || dst.width != (width ? width : 1) ||
        dst.height != (height ? height : 1) ||
        UInt64(row) < UInt64(dst.width) * bpp)
      return false;
    const UInt64 required =
        UInt64(row) * (dst.height - 1) + UInt64(dst.width) * bpp;
    return required <= src.size &&
           UInt64(src.offset) + required <= src.buffer->getCreateInfo().size;
  }
};
} // namespace sinen::gpu
#endif
