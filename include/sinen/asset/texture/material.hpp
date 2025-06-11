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

  void SetTexture(const Texture &texture) {
    if (textures.empty()) {
      textures.push_back(texture);
      return;
    }
    textures[0] = texture;
  }

  void SetTexture(const Texture &texture, const size_t index) {
    if (index >= textures.size()) {
      textures.resize(index + 1);
    }
    textures[index] = texture;
  }

  [[nodiscard]] size_t GetTextureCount() const { return textures.size(); }

  [[nodiscard]] bool HasTexture() const { return !textures.empty(); }

  [[nodiscard]] bool HasTexture(const size_t index) const {
    return index < textures.size();
  }

  void AppendTexture(const Texture &texture) { textures.push_back(texture); }

  [[nodiscard]] const std::vector<Texture> &GetTextures() const {
    return textures;
  }

  void Clear() { textures.clear(); }

  const Texture &GetTexture(const size_t index) { return textures[index]; }

private:
  std::vector<Texture> textures;
};
} // namespace sinen

#endif // SINEN_MATERIAL_HPP
