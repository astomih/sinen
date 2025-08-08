#ifndef SINEN_TRANSFORM_HPP
#define SINEN_TRANSFORM_HPP

#include "../math.hpp"

namespace sinen {
class Transform2D {
public:
  Transform2D() : position(0.0f, 0.0f), rotation(0.0f), scale(1.0f, 1.0f) {}
  ~Transform2D() = default;
  [[nodiscard]] glm::mat4 GetWorldMatrix() const;

  void SetPosition(glm::vec2 position) { this->position = position; }
  void SetRotation(float rotation) { this->rotation = rotation; } // in degrees
  void SetScale(glm::vec2 scale) { this->scale = scale; }

  glm::vec2 position;
  float rotation; // in degrees
  glm::vec2 scale;
};
class Transform3D {
public:
  Transform3D() : position(0.0f), rotation(0.0f), scale(1.0f) {}
  ~Transform3D() = default;
  [[nodiscard]] glm::mat4 GetWorldMatrix() const;

  void SetPosition(glm::vec3 position) { this->position = position; }
  void SetRotation(glm::vec3 rotation) { this->rotation = rotation; }
  void SetScale(glm::vec3 scale) { this->scale = scale; }

  [[nodiscard]] glm::vec3 GetPosition() const { return position; }
  [[nodiscard]] glm::vec3 GetRotation() const { return rotation; }
  [[nodiscard]] glm::vec3 GetScale() const { return scale; }

  glm::vec3 position;
  glm::vec3 rotation;
  glm::vec3 scale;
};

} // namespace sinen

#endif // SINEN_TRANSFORM_HPP
