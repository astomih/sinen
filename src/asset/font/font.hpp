#ifndef SINEN_FONT_HPP
#define SINEN_FONT_HPP
#include <asset/texture/texture.hpp>
#include <geometry/mesh.hpp>


#include <core/data/string.hpp>
namespace sinen {
/**
 * @brief font load and render to texture.
 *
 */
class Font {
public:
  Font();
  Font(int32_t point, StringView file_name);
  ~Font();

  bool load(int point_size);
  bool load(int pointSize, StringView path);
  bool loadFromPath(int pointSize, StringView path);
  bool isLoaded() { return font != nullptr; }
  void unload();
  int size() const { return m_size; }
  void resize(int point_size);

  Texture getAtlas() const;
  Mesh getTextMesh(StringView text) const;

private:
  struct Wrapper;
  UniquePtr<Wrapper> font;
  int m_size;
};
} // namespace sinen
#endif // !SINEN_FONT_HPP
