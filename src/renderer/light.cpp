#include <renderer/light.hpp>

#include <algorithm>
#include <cmath>

namespace sinen {
namespace {

Vec3 normalizedOr(const Vec3 &direction, const Vec3 &fallback) {
  const double length = std::hypot(static_cast<double>(direction.x),
                                   static_cast<double>(direction.y),
                                   static_cast<double>(direction.z));
  if (!std::isfinite(length) || length == 0.0) {
    return fallback;
  }
  return Vec3(static_cast<float>(direction.x / length),
              static_cast<float>(direction.y / length),
              static_cast<float>(direction.z / length));
}

float nonnegativeIntensity(float intensity) {
  return std::isfinite(intensity) ? std::max(0.0f, intensity) : 0.0f;
}

} // namespace

DirectionalLight::DirectionalLight(const Vec3 &direction, float intensity,
                                   const Color &color)
    : intensity(nonnegativeIntensity(intensity)), color(color) {
  setDirection(direction);
}

void DirectionalLight::setDirection(const Vec3 &value) {
  direction = normalizedOr(value, direction);
}

void DirectionalLight::setIntensity(float value) {
  intensity = nonnegativeIntensity(value);
}

PointLight::PointLight(const Vec3 &position, float intensity,
                       const Color &color)
    : position(position), intensity(nonnegativeIntensity(intensity)),
      color(color) {}

void PointLight::setIntensity(float value) {
  intensity = nonnegativeIntensity(value);
}

SpotLight::SpotLight(const Vec3 &position, const Vec3 &direction,
                     float innerAngle, float outerAngle, float intensity,
                     const Color &color)
    : point(position, intensity, color) {
  setDirection(direction);
  setConeAngles(innerAngle, outerAngle);
}

void SpotLight::setDirection(const Vec3 &value) {
  direction = normalizedOr(value, direction);
}

void SpotLight::setConeAngles(float inner, float outer) {
  if (std::isfinite(inner) && std::isfinite(outer)) {
    outerAngle = std::clamp(outer, 0.0f, 90.0f);
    innerAngle = std::clamp(inner, 0.0f, outerAngle);
  }
}

} // namespace sinen
