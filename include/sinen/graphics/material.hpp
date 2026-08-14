#ifndef SINEN_GRAPHICS_MATERIAL_HPP
#define SINEN_GRAPHICS_MATERIAL_HPP

#include <core/buffer/buffer.hpp>
#include <core/data/hashmap.hpp>
#include <core/data/ptr.hpp>
#include <core/data/string.hpp>
#include <graphics/graphics_pipeline.hpp>
#include <graphics/texture/texture.hpp>

namespace sinen {
namespace gpu {
class AccelerationStructure;
}

class Graphics;

// Reusable draw state. A material is independent from a render pass and is
// snapshotted into the command stream when a draw call is encoded.
class Material {
public:
  static constexpr const char *metaTableName() { return "sn.Material"; }

  explicit Material(const GraphicsPipeline &pipeline);

  const GraphicsPipeline &getGraphicsPipeline() const { return pipeline; }

  void setUniformBuffer(UInt32 slotIndex, const Buffer &buffer);
  void setUniformBuffer(StringView name, const Buffer &buffer);
  void resetUniformBuffer(UInt32 slotIndex);
  void resetUniformBuffer(StringView name);
  void resetAllUniformBuffers();

  void setTexture(UInt32 slotIndex, const Ptr<Texture> &texture);
  void setTexture(StringView name, const Ptr<Texture> &texture);
  void resetTexture(UInt32 slotIndex);
  void resetTexture(StringView name);
  void resetAllTexture();

  void setAccelerationStructure(
      UInt32 slotIndex,
      const Ptr<gpu::AccelerationStructure> &accelerationStructure);
  void resetAccelerationStructure(UInt32 slotIndex);
  void resetAllAccelerationStructures();

private:
  friend class Graphics;

  bool resolveUniformBufferSlot(StringView name, UInt32 &slot) const;
  bool resolveTextureSlot(StringView name, UInt32 &slot) const;

  GraphicsPipeline pipeline;
  Hashmap<UInt32, Buffer> uniformBufferBindings;
  Hashmap<UInt32, Ptr<Texture>> textureBindings;
  Hashmap<UInt32, Ptr<gpu::AccelerationStructure>>
      accelerationStructureBindings;
};
} // namespace sinen

#endif // SINEN_GRAPHICS_MATERIAL_HPP
