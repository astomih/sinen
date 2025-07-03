#ifndef SINEN_TRANSFORM_HPP
#define SINEN_TRANSFORM_HPP

#include "../math.hpp"

namespace sinen {
class Transform2D {
public:
  Transform2D() : position(0.0f, 0.0f), rotation(0.0f), scale(1.0f, 1.0f) {}
  ~Transform2D() = default;
  void SetPosition(const glm::vec2 &position) { this->position = position; }
  [[nodiscard]] const glm::vec2 &GetPosition() const { return this->position; }
  void SetRotation(float rotation) { this->rotation = rotation; }
  [[nodiscard]] float GetRotation() const { return this->rotation; }

  void SetScale(const glm::vec2 &scale) { this->scale = scale; }
  [[nodiscard]] const glm::vec2 &GetScale() const { return this->scale; }

  [[nodiscard]] glm::mat4 GetWorldMatrix() const;

  glm::vec2 position;
  float rotation; // in degrees
  glm::vec2 scale;
};
class Transform3D {
public:
  Transform3D() : position(0.0f), rotation(0.0f), scale(1.0f) {}
  ~Transform3D() = default;
  void SetPosition(const glm::vec3 &position) { this->position = position; }
  [[nodiscard]] const glm::vec3 &GetPosition() const { return this->position; }

  void SetRotation(const glm::vec3 &rotation) { this->rotation = rotation; }
  [[nodiscard]] const glm::vec3 &GetRotation() const { return rotation; }

  void SetScale(const glm::vec3 &scale) { this->scale = scale; }
  [[nodiscard]] const glm::vec3 &GetScale() const { return this->scale; }

  [[nodiscard]] glm::mat4 GetWorldMatrix() const;

  glm::vec3 position;
  glm::vec3 rotation;
  glm::vec3 scale;
};

} // namespace sinen

#endif // SINEN_TRANSFORM_HPP
