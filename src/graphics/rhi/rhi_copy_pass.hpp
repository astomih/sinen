#ifndef SINEN_RHI_COPY_PASS_HPP
#define SINEN_RHI_COPY_PASS_HPP
#include "rhi_buffer.hpp"
#include <core/data/ptr.hpp>
#include <core/def/types.hpp>

namespace sinen::rhi {
struct TextureRegion {
  Ptr<class Texture> texture;
  uint32 mipLevel;
  uint32 layer;
  uint32 x, y, z;
  uint32 width;
  uint32 height;
  uint32 depth;
};
struct TextureTransferInfo {
  Ptr<class TransferBuffer> transferBuffer;
  uint32 offset;
};
struct TextureLocation {
  Ptr<Texture> texture;
  uint32 mipLevel;
  uint32 layer;
  uint32 x;
  uint32 y;
  uint32 z;
};
struct BufferTransferInfo {
  Ptr<class TransferBuffer> transferBuffer;
  uint32 offset;
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

  virtual void copyTexture(const TextureLocation &src,
                           const TextureLocation &dst, uint32 width,
                           uint32 height, uint32 depth, bool cycle) = 0;
};
} // namespace sinen::rhi
#endif