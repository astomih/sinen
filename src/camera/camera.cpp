#include "../manager/get_system.hpp"
#include "../window/window_system.hpp"
#include <camera/camera.hpp>
#include <math/matrix4.hpp>
#include <render/renderer.hpp>
#include <window/window.hpp>

namespace sinen {
camera::camera()
    : fov(math::to_radians(70.f)),
      aspect(get_window().size.x / get_window().size.y), near(0.1f),
      far(1000.f), position(vector3(0.f, 0.f, 10.f)), target(0.f, 0.f, 0.f),
      up(vector3(0, -1.f, 0)) {
  update();
}

void camera::update() {
  view = matrix4::lookat(this->position, this->target, this->up);
  projection =
      matrix4::perspective(this->fov, this->aspect, this->near, this->far);
}
} // namespace sinen
