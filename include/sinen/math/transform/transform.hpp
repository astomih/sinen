#ifndef SINEN_TRANSFORM_HPP
#define SINEN_TRANSFORM_HPP

#include <math/matrix.hpp>
#include <math/vector.hpp>
namespace sinen {
class Transform {
public:
  Transform() : position(0.0f), rotation(0.0f), scale(1.0f) {}
  ~Transform() = default;
  static constexpr const char *metaTableName() { return "sn.Transform"; }
  String tableString() const;
  [[nodiscard]] Mat4 getWorldMatrix() const;

  void setPosition(Vec3 position) { this->position = position; }
  void setRotation(Vec3 rotation) { this->rotation = rotation; }
  void setScale(Vec3 scale) { this->scale = scale; }

  [[nodiscard]] Vec3 getPosition() const { return position; }
  [[nodiscard]] Vec3 getRotation() const { return rotation; }
  [[nodiscard]] Vec3 getScale() const { return scale; }

  Vec3 position;
  Vec3 rotation;
  Vec3 scale;
};

} // namespace sinen

#endif // SINEN_TRANSFORM_HPP
