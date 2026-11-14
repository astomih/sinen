#ifndef SINEN_RENDERER_LIGHT_HPP
#define SINEN_RENDERER_LIGHT_HPP

#include <math/color/color.hpp>
#include <math/vec3.hpp>

#include <variant>

namespace sinen {

class DirectionalLight {
public:
  static constexpr const char *metaTableName() { return "sn.DirectionalLight"; }

  // World-space direction in which the light travels.
  explicit DirectionalLight(const Vec3 &direction = Vec3(0, 0, -1),
                            float intensity = 1.0f,
                            const Color &color = Color(1.0f));
  void setDirection(const Vec3 &direction);
  void setIntensity(float intensity);
  void setColor(const Color &color) { this->color = color; }
  const Vec3 &getDirection() const { return direction; }
  float getIntensity() const { return intensity; }
  const Color &getColor() const { return color; }

private:
  Vec3 direction{0, 0, -1};
  float intensity = 1.0f;
  Color color{1.0f};
};

class PointLight {
public:
  static constexpr const char *metaTableName() { return "sn.PointLight"; }

  explicit PointLight(const Vec3 &position = Vec3(0), float intensity = 4.0f,
                      const Color &color = Color(1.0f));
  void setPosition(const Vec3 &position) { this->position = position; }
  void setIntensity(float intensity);
  void setColor(const Color &color) { this->color = color; }
  const Vec3 &getPosition() const { return position; }
  float getIntensity() const { return intensity; }
  const Color &getColor() const { return color; }

private:
  Vec3 position;
  float intensity;
  Color color;
};

class SpotLight {
public:
  static constexpr const char *metaTableName() { return "sn.SpotLight"; }

  // Cone angles are half-angles in degrees measured from the emission axis.
  explicit SpotLight(const Vec3 &position = Vec3(0),
                     const Vec3 &direction = Vec3(0, 0, -1),
                     float innerAngle = 20.0f, float outerAngle = 35.0f,
                     float intensity = 4.0f, const Color &color = Color(1.0f));
  void setPosition(const Vec3 &position) { point.setPosition(position); }
  // World-space direction in which the light travels.
  void setDirection(const Vec3 &direction);
  // Enforce 0 <= innerAngle <= outerAngle <= 90 degrees.
  void setConeAngles(float innerAngle, float outerAngle);
  void setIntensity(float intensity) { point.setIntensity(intensity); }
  void setColor(const Color &color) { point.setColor(color); }
  const Vec3 &getPosition() const { return point.getPosition(); }
  const Vec3 &getDirection() const { return direction; }
  float getInnerAngle() const { return innerAngle; }
  float getOuterAngle() const { return outerAngle; }
  float getIntensity() const { return point.getIntensity(); }
  const Color &getColor() const { return point.getColor(); }

private:
  PointLight point;
  Vec3 direction{0, 0, -1};
  float innerAngle = 20.0f;
  float outerAngle = 35.0f;
};

using Light = std::variant<DirectionalLight, PointLight, SpotLight>;

} // namespace sinen

#endif
