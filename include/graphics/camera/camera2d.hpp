#ifndef SINEN_CAMERA2D_HPP
#define SINEN_CAMERA2D_HPP
#include <math/geometry/rect.hpp>
#include <math/vector.hpp>
namespace sinen {
class Camera2D {
public:
  Camera2D() = default;
  Camera2D(const Vec2 &size);
  ~Camera2D() = default;

  static constexpr const char *metaTableName() { return "sn.Camera2D"; }

  void resize(const Vec2 &size);
  Vec2 windowRatio() const;
  Vec2 invWindowRatio() const;
  Vec2 size() const;
  Vec2 half() const;
  Rect rect() const;
  Vec2 topLeft() const;
  Vec2 topCenter() const;
  Vec2 topRight() const;
  Vec2 left() const;
  Vec2 center() const;
  Vec2 right() const;
  Vec2 bottomLeft() const;
  Vec2 bottomCenter() const;
  Vec2 bottomRight() const;

private:
  Rect rect_;
};
} // namespace sinen

#endif // SINEN_CAMERA2D_HPP
