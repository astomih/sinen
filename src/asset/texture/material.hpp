#ifndef SINEN_MATERIAL_HPP
#define SINEN_MATERIAL_HPP
#include "texture.hpp"
#include <core/data/hashmap.hpp>
#include <graphics/graphics_pipeline.hpp>

namespace sinen {
/**
 * @brief Material
 *
 */
class Material {
public:
  Material() = default;
  explicit Material(const Texture &texture) { textures.push_back(texture); }

  ~Material() = default;

  void setTexture(const Texture &texture) {
    if (textures.empty()) {
      textures.push_back(texture);
      return;
    }
    textures[0] = texture;
  }
  void insertTexture(const Texture &texture, Size index);

  void setTexture(const Texture &texture, Size index) {
    if (index >= textures.size()) {
      textures.resize(index + 1);
    }
    textures[index] = texture;
  }

  [[nodiscard]] size_t getTextureCount() const { return textures.size(); }

  [[nodiscard]] bool hasTexture() const { return !textures.empty(); }

  [[nodiscard]] bool hasTexture(Size index) const {
    return index < textures.size();
  }

  void appendTexture(const Texture &texture) { textures.push_back(texture); }

  [[nodiscard]] const Array<Texture> &getTextures() const { return textures; }

  void clear() { textures.clear(); }

  const Texture &getTexture(const size_t index) const {
    return textures[index];
  }

  void setGraphicsPipeline(const GraphicsPipeline &pipeline);
  std::optional<GraphicsPipeline> getGraphicsPipeline() const {
    return gfxPipeline;
  }

  void setUniformBuffer(UInt32 slotIndex, const Buffer &buffer);
  std::optional<Buffer> getUniformBuffer(UInt32 slotIndex) const;
  const Hashmap<UInt32, Buffer> &getUniformBuffers() const {
    return uniformBuffer;
  }

private:
  Array<Texture> textures;
  std::optional<GraphicsPipeline> gfxPipeline;
  Hashmap<UInt32, Buffer> uniformBuffer;
};
} // namespace sinen

#endif // SINEN_MATERIAL_HPP
