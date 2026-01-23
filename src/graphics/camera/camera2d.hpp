#ifndef SINEN_CAMERA2D_HPP
#define SINEN_CAMERA2D_HPP
#include <math/vector.hpp>
namespace sinen {
class Camera2D {
public:
  Camera2D() = default;
  Camera2D(const Vec2 &size) : cameraSize(size) {}
  ~Camera2D() = default;

  static constexpr const char *metaTableName() { return "sn.Camera2D"; }

  void resize(const Vec2 &size) { this->cameraSize = size; }
  Vec2 windowRatio() const;
  Vec2 invWindowRatio() const;
  Vec2 size() const { return cameraSize; }
  Vec2 half() const { return cameraSize * 0.5f; }

private:
  Vec2 cameraSize;
};
} // namespace sinen

#endif // SINEN_CAMERA2D_HPP
