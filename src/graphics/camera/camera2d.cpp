#include <graphics/camera/camera2d.hpp>
#include <platform/window/window.hpp>

#include <math/vector.hpp>

namespace sinen {

Vec2 Camera2D::windowRatio() const { return Window::size() / cameraSize; }
Vec2 Camera2D::invWindowRatio() const { return cameraSize / Window::size(); }
} // namespace sinen