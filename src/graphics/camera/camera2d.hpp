#ifndef SINEN_CAMERA2D_HPP
#define SINEN_CAMERA2D_HPP
#include <glm/vec2.hpp>
namespace sinen {
class Camera2D {
public:
  Camera2D() = default;
  Camera2D(const glm::vec2 &size) : cameraSize(size) {}
  ~Camera2D() = default;

  void resize(const glm::vec2 &size) { this->cameraSize = size; }
  glm::vec2 windowRatio() const;
  glm::vec2 invWindowRatio() const;
  glm::vec2 size() const { return cameraSize; }
  glm::vec2 half() const { return cameraSize * 0.5f; }

private:
  glm::vec2 cameraSize;
};
} // namespace sinen

#endif // SINEN_CAMERA2D_HPP