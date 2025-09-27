#ifndef SINEN_MATERIAL_HPP
#define SINEN_MATERIAL_HPP
#include "texture.hpp"

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

  void setTexture(const Texture &texture, const size_t index) {
    if (index >= textures.size()) {
      textures.resize(index + 1);
    }
    textures[index] = texture;
  }

  [[nodiscard]] size_t getTextureCount() const { return textures.size(); }

  [[nodiscard]] bool hasTexture() const { return !textures.empty(); }

  [[nodiscard]] bool hasTexture(const size_t index) const {
    return index < textures.size();
  }

  void appendTexture(const Texture &texture) { textures.push_back(texture); }

  [[nodiscard]] const std::vector<Texture> &getTextures() const {
    return textures;
  }

  void clear() { textures.clear(); }

  const Texture &getTexture(const size_t index) { return textures[index]; }

private:
  std::vector<Texture> textures;
};
} // namespace sinen

#endif // SINEN_MATERIAL_HPP
