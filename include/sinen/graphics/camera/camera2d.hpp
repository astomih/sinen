#ifndef SINEN_CAMERA2D_HPP
#define SINEN_CAMERA2D_HPP
#include <glm/vec2.hpp>
namespace sinen {
class Camera2D {
public:
  Camera2D() = default;
  Camera2D(const glm::vec2 &size) : size(size) {}
  ~Camera2D() = default;

  void Resize(const glm::vec2 &size) { this->size = size; }
  glm::vec2 WindowRatio() const;
  glm::vec2 InvWindowRatio() const;
  glm::vec2 Size() const { return size; }
  glm::vec2 Half() const { return size * 0.5f; }

private:
  glm::vec2 size;
};
} // namespace sinen

#endif // SINEN_CAMERA2D_HPP