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

  void append(const Texture &texture) { textures.push_back(texture); }

  [[nodiscard]] const std::vector<Texture> &get_textures() const {
    return textures;
  }

  void clear() { textures.clear(); }

  const Texture &get_texture(const size_t index) { return textures[index]; }

private:
  std::vector<Texture> textures;
};
} // namespace sinen

#endif // SINEN_MATERIAL_HPP
