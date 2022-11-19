#include "../window/window_system.hpp"
#include <camera/camera.hpp>
#include <logger/logger.hpp>
#include <math/matrix4.hpp>
#include <render/renderer.hpp>
#include <window/window.hpp>

namespace sinen {

vector3 camera::m_position = vector3(0.f, -1.f, 10.f);
vector3 camera::m_target = vector3(0.f, 1.f, 0.f);
vector3 camera::m_up = vector3(0.f, 0.f, 1.f);
matrix4 camera::m_view = matrix4::lookat(m_position, m_target, m_up);
matrix4 camera::m_projection =
    matrix4::perspective(math::to_radians(70.f), 1280.f / 720.f, .1f, 1000.f);
void camera::sub::lookat(const vector3 &position, const vector3 &target,
                         const vector3 &up) {
  m_position = position;
  m_target = target;
  m_up = up;
  m_view = matrix4::lookat(m_position, m_target, m_up);
}
void camera::sub::perspective(float fov, float aspect, float near, float far) {
  m_projection = matrix4::perspective(math::to_radians(fov), aspect, near, far);
}
void camera::sub::orthographic(float width, float height, float near,
                               float far) {
  m_projection = matrix4::ortho(width, height, near, far);
}
void camera::lookat(const vector3 &position, const vector3 &target,
                    const vector3 &up) {
  m_position = position;
  m_target = target;
  m_up = up;
  m_view = matrix4::lookat(m_position, m_target, m_up);
}
void camera::perspective(float fov, float aspect, float near, float far) {
  m_projection = matrix4::perspective(math::to_radians(fov), aspect, near, far);
}
void camera::orthographic(float width, float height, float near, float far) {
  m_projection = matrix4::ortho(width, height, near, far);
}

camera::sub camera::create_sub_camera_from_init() {
  sub obj;
  obj.m_position = vector3(0.f, -1.f, 10.f);
  obj.m_target = vector3(0.f, 1.f, 0.f);
  obj.m_up = vector3(0.f, 0.f, 1.f);
  obj.m_view = matrix4::lookat(obj.m_position, obj.m_target, obj.m_up);
  obj.m_projection = matrix4::perspective(
      math::to_radians(70.f), window::size().x / window::size().y, .1f, 1000.f);
  return obj;
}
camera::sub camera::create_sub_camera_from_main() {
  sub obj;
  memcpy(&obj, &camera::m_position, sizeof(camera::sub));
  return obj;
}

} // namespace sinen