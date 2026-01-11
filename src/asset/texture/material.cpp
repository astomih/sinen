#include "material.hpp"

namespace sinen {
void Material::insertTexture(const Texture &texture, Size index) {
  this->textures.insert(this->textures.begin() + index, texture);
}
void Material::setGraphicsPipeline(const GraphicsPipeline &pipeline) {
  this->gfxPipeline.emplace(pipeline);
}
void Material::setUniformBuffer(UInt32 slotIndex, const Buffer &buffer) {
  this->uniformBuffer.insert_or_assign(slotIndex, buffer);
}
std::optional<Buffer> Material::getUniformBuffer(UInt32 slotIndex) const {
  if (uniformBuffer.contains(slotIndex)) {
    return uniformBuffer.at(slotIndex);
  }
  return std::nullopt;
}
} // namespace sinen