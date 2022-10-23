#include "../render/render_system.hpp"
#include "../window/window_system.hpp"
#include <camera/camera.hpp>
#include <drawable/drawable_wrapper.hpp>
#include <render/renderer.hpp>
#include <window/window.hpp>

namespace sinen {
draw2d::draw2d()
    : position(vector2(0.f, 0.f)), rotation(0.0f), scale(vector2(1.f, 1.f)) {}
draw2d::draw2d(texture texture_handle)
    : position(vector2(0.f, 0.f)), rotation(0.0f), scale(vector2(1.f, 1.f)),
      texture_handle(texture_handle) {}
draw3d::draw3d()
    : position(vector3(0.f, 0.f, 0.f)), rotation(vector3(0.f, 0.f, 0.f)),
      scale(vector3(1.f, 1.f, 1.f)) {}
draw3d::draw3d(texture texture_handle)
    : position(vector3(0.f, 0.f, 0.f)), rotation(vector3(0.f, 0.f, 0.f)),
      scale(vector3(1.f, 1.f, 1.f)), texture_handle(texture_handle) {}
void draw2d::draw() {
  auto obj = std::make_shared<drawable>();
  matrix4 t = matrix4::identity;
  t.mat[3][0] = position.x;
  t.mat[3][1] = position.y;
  quaternion q(vector3::neg_unit_z, rotation);
  matrix4 r = matrix4::create_from_quaternion(q);
  matrix4 s = matrix4::identity;
  s.mat[0][0] = scale.x * 0.5f;
  s.mat[1][1] = scale.y * 0.5f;
  obj->param.world = s * r * t;
  obj->texture_handle = this->texture_handle;
  matrix4 viewproj = matrix4::identity;

  auto windowsize = window::size();
  viewproj.mat[0][0] = 2.f / windowsize.x;
  viewproj.mat[1][1] = 2.f / windowsize.y;
  obj->param.proj = viewproj;
  obj->param.view = matrix4::identity;
  obj->vertexIndex = this->vertex_name;
  renderer::draw2d(obj);
  *this->texture_handle.is_need_update = false;
}
void draw3d::draw() {
  auto obj = std::make_shared<drawable>();
  obj->texture_handle = this->texture_handle;
  matrix4 t = matrix4::identity;
  t.mat[3][0] = position.x;
  t.mat[3][1] = position.y;
  t.mat[3][2] = position.z;
  quaternion q;
  q = quaternion::concatenate(
      q, quaternion(vector3::unit_z, math::to_radians(rotation.z)));
  q = quaternion::concatenate(
      q, quaternion(vector3::unit_y, math::to_radians(rotation.y)));
  q = quaternion::concatenate(
      q, quaternion(vector3::unit_x, math::to_radians(rotation.x)));
  matrix4 r = matrix4::create_from_quaternion(q);
  matrix4 s = matrix4::identity;
  s.mat[0][0] = scale.x;
  s.mat[1][1] = scale.y;
  s.mat[2][2] = scale.z;
  obj->param.world = s * r * t;
  obj->param.proj = camera::projection();
  obj->param.view = camera::view();
  obj->vertexIndex = this->vertex_name;
  obj->is_draw_depth = this->is_draw_depth;
  renderer::draw3d(obj);
  *this->texture_handle.is_need_update = false;
}
} // namespace sinen
