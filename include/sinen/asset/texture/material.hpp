#ifndef SINEN_MATERIAL_HPP
#define SINEN_MATERIAL_HPP
#include "cubemap.hpp"
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

  void setCubemap(const Cubemap &cubemap) {
    if (cubemaps.empty()) {
      cubemaps.push_back(cubemap);
      return;
    }
    cubemaps[0] = cubemap;
  }

  void setCubemap(const Cubemap &cubemap, const size_t index) {
    if (index >= cubemaps.size()) {
      cubemaps.resize(index + 1);
    }
    cubemaps[index] = cubemap;
  }

  [[nodiscard]] size_t getTextureCount() const { return textures.size(); }

  [[nodiscard]] bool hasTexture() const { return !textures.empty(); }
  [[nodiscard]] bool hasCubemap() const { return !cubemaps.empty(); }

  [[nodiscard]] bool hasTexture(const size_t index) const {
    return index < textures.size();
  }

  void appendTexture(const Texture &texture) { textures.push_back(texture); }

  [[nodiscard]] const std::vector<Texture> &getTextures() const {
    return textures;
  }

  [[nodiscard]] const std::vector<Cubemap> &getCubemaps() const {
    return cubemaps;
  }

  void clear() { textures.clear(); }

  const Texture &getTexture(const size_t index) const {
    return textures[index];
  }
  const Cubemap &getCubemap(const size_t index) const {
    return cubemaps[index];
  }

private:
  std::vector<Texture> textures;
  std::vector<Cubemap> cubemaps;
};
} // namespace sinen

#endif // SINEN_MATERIAL_HPP
