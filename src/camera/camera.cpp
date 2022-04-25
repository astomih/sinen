#include <manager/manager.hpp>

#include <camera/camera.hpp>
#include <math/matrix4.hpp>
#include <render/renderer.hpp>
#include <window/window.hpp>

namespace nen {
camera::camera()
    : fov(math::to_radians(70.f)),
      aspect(get_window().Size().x / get_window().Size().y), near(0.1f),
      far(1000.f), position(vector3(0.f, 0.f, 10.f)), target(0.f, 0.f, 0.f),
      up(vector3(0, -1.f, 0)) {
  update();
}

void camera::update() {
  view = matrix4::LookAt(this->position, this->target, this->up);
  projection =
      matrix4::Perspective(this->fov, this->aspect, this->near, this->far);
}
} // namespace nen