#include <DrawObject/draw_object_wrapper.hpp>
#include <Render/Renderer.hpp>
#include <Window/Window.hpp>
#include <camera/camera.hpp>
#include <manager/manager.hpp>

namespace nen {
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
  auto obj = std::make_shared<draw_object>();
  matrix4 t = matrix4::Identity;
  t.mat[3][0] = position.x;
  t.mat[3][1] = position.y;
  quaternion q(vector3::NegUnitZ, rotation);
  matrix4 r = matrix4::CreateFromQuaternion(q);
  matrix4 s = matrix4::Identity;
  s.mat[0][0] = scale.x * 0.5f;
  s.mat[1][1] = scale.y * 0.5f;
  obj->param.world = s * r * t;
  obj->texture_handle = texture_handle.handle;
  matrix4 viewproj = matrix4::Identity;

  auto windowsize = get_window().Size();
  viewproj.mat[0][0] = 2.f / windowsize.x;
  viewproj.mat[1][1] = 2.f / windowsize.y;
  obj->param.proj = viewproj;
  obj->param.view = matrix4::Identity;
  obj->vertexIndex = this->vertex_name;
  get_renderer().draw2d(obj);
}
void draw3d::draw() {
  auto obj = std::make_shared<draw_object>();
  matrix4 t = matrix4::Identity;
  t.mat[3][0] = position.x;
  t.mat[3][1] = position.y;
  t.mat[3][2] = position.z;
  quaternion q;
  q = quaternion::Concatenate(
      q, quaternion(vector3::UnitZ, Math::ToRadians(rotation.z)));
  q = quaternion::Concatenate(
      q, quaternion(vector3::UnitY, Math::ToRadians(rotation.y)));
  q = quaternion::Concatenate(
      q, quaternion(vector3::UnitX, Math::ToRadians(rotation.x)));
  matrix4 r = matrix4::CreateFromQuaternion(q);
  matrix4 s = matrix4::Identity;
  s.mat[0][0] = scale.x;
  s.mat[1][1] = scale.y;
  s.mat[2][2] = scale.z;
  obj->param.world = s * r * t;
  obj->texture_handle = texture_handle.handle;
  obj->param.proj = get_camera().get_projection();
  obj->param.view = get_camera().get_view();
  obj->vertexIndex = this->vertex_name;
  get_renderer().draw3d(obj);
}
} // namespace nen