#include <graphics/camera/camera2d.hpp>
#include <platform/window/window.hpp>

#include <math/vector.hpp>

namespace sinen {
Camera2D::Camera2D(const Vec2 &size) : rect_(Vec2(0, 0), size) {}
void Camera2D::resize(const Vec2 &size) {
  this->rect_.width = size.x;
  this->rect_.height = size.y;
}
Vec2 Camera2D::windowRatio() const { return Window::size() / rect_.size(); }
Vec2 Camera2D::invWindowRatio() const { return rect_.size() / Window::size(); }
Vec2 Camera2D::size() const { return rect_.size(); }
Vec2 Camera2D::half() const { return rect_.size() * 0.5f; }
Rect Camera2D::rect() const { return Rect(0.0f, 0.0f, size().x, size().y); }
Vec2 Camera2D::topLeft() const { return rect().topLeft(); }
Vec2 Camera2D::topCenter() const { return rect().topCenter(); }
Vec2 Camera2D::topRight() const { return rect().topRight(); }
Vec2 Camera2D::left() const { return rect().left(); }
Vec2 Camera2D::center() const { return rect().center(); }
Vec2 Camera2D::right() const { return rect().right(); }
Vec2 Camera2D::bottomLeft() const { return rect().bottomLeft(); }
Vec2 Camera2D::bottomCenter() const { return rect().bottomCenter(); }
Vec2 Camera2D::bottomRight() const { return rect().bottomRight(); }

} // namespace sinen
