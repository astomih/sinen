#ifndef SINEN_TRANSFORM_HPP
#define SINEN_TRANSFORM_HPP

#include <math/matrix.hpp>
#include <math/vector.hpp>
namespace sinen {
class Transform {
public:
  Transform() : position(0.0f), rotation(0.0f), scale(1.0f) {}
  ~Transform() = default;
  [[nodiscard]] glm::mat4 getWorldMatrix() const;

  void setPosition(glm::vec3 position) { this->position = position; }
  void setRotation(glm::vec3 rotation) { this->rotation = rotation; }
  void setScale(glm::vec3 scale) { this->scale = scale; }

  [[nodiscard]] glm::vec3 getPosition() const { return position; }
  [[nodiscard]] glm::vec3 getRotation() const { return rotation; }
  [[nodiscard]] glm::vec3 getScale() const { return scale; }

  glm::vec3 position;
  glm::vec3 rotation;
  glm::vec3 scale;
};

} // namespace sinen

#endif // SINEN_TRANSFORM_HPP
