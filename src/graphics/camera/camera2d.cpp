#include <graphics/camera/camera2d.hpp>
#include <platform/window/window.hpp>

namespace sinen {

glm::vec2 Camera2D::windowRatio() const { return Window::size() / cameraSize; }
glm::vec2 Camera2D::invWindowRatio() const { return cameraSize / Window::size(); }
} // namespace sinen