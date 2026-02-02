#ifndef SINEN_FONT_HPP
#define SINEN_FONT_HPP
#include <asset/texture/texture.hpp>
#include <core/data/string.hpp>
#include <math/geometry/mesh.hpp>
#include <math/geometry/rect.hpp>

namespace sinen {
/**
 * @brief font load and render to texture.
 *
 */
class Font {
public:
  static Ptr<Font> create();
  static Ptr<Font> create(int32_t point, StringView file_name);
  virtual ~Font() = default;

  static constexpr const char *metaTableName() { return "sn.Font"; }

  virtual bool load(int point_size) = 0;
  virtual bool load(int pointSize, StringView path) = 0;
  virtual bool load(int pointSize, const Buffer &buffer) = 0;
  virtual bool isLoaded() = 0;
  virtual void unload() = 0;
  virtual int size() const = 0;
  virtual void resize(int point_size) = 0;
  virtual Rect region(StringView text, int fontSize, const Pivot &pivot,
                      const Vec2 &vec) = 0;
  virtual Ptr<Texture> getAtlas() const = 0;
  virtual Mesh getTextMesh(StringView text) const = 0;
};
} // namespace sinen
#endif // !SINEN_FONT_HPP
