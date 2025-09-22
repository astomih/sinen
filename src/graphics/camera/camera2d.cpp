#include <graphics/camera/camera2d.hpp>
#include <platform/window/window.hpp>

namespace sinen {

glm::vec2 Camera2D::WindowRatio() const { return Window::Size() / size; }
glm::vec2 Camera2D::InvWindowRatio() const { return size / Window::Size(); }
} // namespace sinen