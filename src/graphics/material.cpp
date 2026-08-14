#include <core/logger/log.hpp>
#include <graphics/material.hpp>

namespace sinen {
Material::Material(const GraphicsPipeline &pipeline) : pipeline(pipeline) {
  if (!pipeline.get()) {
    Log::error("Material was created with an unbuilt graphics pipeline");
  }
}

bool Material::resolveUniformBufferSlot(StringView name, UInt32 &slot) const {
  if (!pipeline.get()) {
    Log::error("Material uniform binding requires a built graphics pipeline");
    return false;
  }
  if (!pipeline.findUniformBufferSlot(name, slot)) {
    Log::error("Uniform buffer '%.*s' was not found in the material pipeline",
               static_cast<int>(name.size()), name.data());
    return false;
  }
  return true;
}

bool Material::resolveTextureSlot(StringView name, UInt32 &slot) const {
  if (!pipeline.get()) {
    Log::error("Material texture binding requires a built graphics pipeline");
    return false;
  }
  if (!pipeline.findTextureSlot(name, slot)) {
    Log::error("Texture '%.*s' was not found in the material pipeline",
               static_cast<int>(name.size()), name.data());
    return false;
  }
  return true;
}

void Material::setUniformBuffer(UInt32 slotIndex, const Buffer &buffer) {
  uniformBufferBindings.insert_or_assign(slotIndex, buffer);
}

void Material::setUniformBuffer(StringView name, const Buffer &buffer) {
  UInt32 slot = 0;
  if (resolveUniformBufferSlot(name, slot)) {
    setUniformBuffer(slot, buffer);
  }
}

void Material::resetUniformBuffer(UInt32 slotIndex) {
  uniformBufferBindings.erase(slotIndex);
}

void Material::resetUniformBuffer(StringView name) {
  UInt32 slot = 0;
  if (resolveUniformBufferSlot(name, slot)) {
    resetUniformBuffer(slot);
  }
}

void Material::resetAllUniformBuffers() { uniformBufferBindings.clear(); }

void Material::setTexture(UInt32 slotIndex, const Ptr<Texture> &texture) {
  textureBindings.insert_or_assign(slotIndex, texture);
}

void Material::setTexture(StringView name, const Ptr<Texture> &texture) {
  UInt32 slot = 0;
  if (resolveTextureSlot(name, slot)) {
    setTexture(slot, texture);
  }
}

void Material::resetTexture(UInt32 slotIndex) {
  textureBindings.erase(slotIndex);
}

void Material::resetTexture(StringView name) {
  UInt32 slot = 0;
  if (resolveTextureSlot(name, slot)) {
    resetTexture(slot);
  }
}

void Material::resetAllTexture() { textureBindings.clear(); }

void Material::setAccelerationStructure(
    UInt32 slotIndex,
    const Ptr<gpu::AccelerationStructure> &accelerationStructure) {
  accelerationStructureBindings.insert_or_assign(slotIndex,
                                                 accelerationStructure);
}

void Material::resetAccelerationStructure(UInt32 slotIndex) {
  accelerationStructureBindings.erase(slotIndex);
}

void Material::resetAllAccelerationStructures() {
  accelerationStructureBindings.clear();
}
} // namespace sinen
