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

  ~Material() = default;

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
