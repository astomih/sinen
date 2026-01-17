#ifndef SINEN_FONT_HPP
#define SINEN_FONT_HPP
#include <asset/texture/texture.hpp>
#include <core/data/string.hpp>
#include <math/geometry/mesh.hpp>
#include <math/geometry/rect.hpp>

#include <future>
#include <stb_truetype.h>

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
  bool load(int pointSize, const Buffer &buffer);
  bool isLoaded() { return m_size != 0; }
  void unload();
  int size() const { return m_size; }
  void resize(int point_size);

  Rect region(StringView text, int fontSize, float x, float y);

  Texture getAtlas() const;
  Mesh getTextMesh(StringView text) const;

private:
  Array<Array<stbtt_packedchar>> packedChar;
  stbtt_fontinfo fontInfo;
  Texture texture;
  uint32_t sheetSize;
  std::future<bool> future;
  Array<unsigned char> atlasBitmap;
  String data;
  bool loaded = false;
  int m_size;
};
} // namespace sinen
#endif // !SINEN_FONT_HPP
