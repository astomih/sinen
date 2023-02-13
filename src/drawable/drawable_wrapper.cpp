#include <camera/camera.hpp>
#include <drawable/drawable_wrapper.hpp>
#include <render/renderer.hpp>
#include <scene/scene.hpp>
#include <window/window.hpp>

namespace sinen {
draw2d::draw2d()
    : position(vector2(0.f, 0.f)), rotation(0.0f), scale(vector2(1.f, 1.f)) {
  obj = std::make_shared<drawable>();
}
draw2d::draw2d(texture texture_handle)
    : position(vector2(0.f, 0.f)), rotation(0.0f), scale(vector2(1.f, 1.f)),
      texture_handle(texture_handle) {
  obj = std::make_shared<drawable>();
}
drawui::drawui()
    : position(vector2(0.f, 0.f)), rotation(0.0f), scale(vector2(1.f, 1.f)) {
  obj = std::make_shared<drawable>();
}
drawui::drawui(texture texture_handle)
    : position(vector2(0.f, 0.f)), rotation(0.0f), scale(vector2(1.f, 1.f)),
      texture_handle(texture_handle) {
  obj = std::make_shared<drawable>();
}
draw3d::draw3d()
    : position(vector3(0.f, 0.f, 0.f)), rotation(vector3(0.f, 0.f, 0.f)),
      scale(vector3(1.f, 1.f, 1.f)) {
  obj = std::make_shared<drawable>();
}
draw3d::draw3d(texture texture_handle)
    : position(vector3(0.f, 0.f, 0.f)), rotation(vector3(0.f, 0.f, 0.f)),
      scale(vector3(1.f, 1.f, 1.f)), texture_handle(texture_handle) {
  obj = std::make_shared<drawable>();
}
void draw2d::draw() {
  matrix4 t = matrix4::identity;
  t.mat[3][0] = position.x;
  t.mat[3][1] = position.y;
  quaternion q(vector3::neg_unit_z, rotation);
  matrix4 r = matrix4::create_from_quaternion(q);
  matrix4 s = matrix4::identity;
  s.mat[0][0] = scale.x * 0.5f;
  s.mat[1][1] = scale.y * 0.5f;
  obj->param.world = s * r * t;
  obj->binding_texture = this->texture_handle;
  matrix4 viewproj = matrix4::identity;

  auto screen_size = scene::size();
  viewproj.mat[0][0] = 2.f / screen_size.x;
  viewproj.mat[1][1] = 2.f / screen_size.y;
  obj->param.proj = viewproj;
  obj->param.view = matrix4::identity;
  obj->vertexIndex = this->vertex_name;
  for (auto &i : worlds) {
    matrix4 t = matrix4::identity;
    t.mat[3][0] = i.position.x;
    t.mat[3][1] = i.position.y;
    quaternion q(vector3::neg_unit_z, i.rotation);
    matrix4 r = matrix4::create_from_quaternion(q);
    matrix4 s = matrix4::identity;
    s.mat[0][0] = i.scale.x * 0.5f;
    s.mat[1][1] = i.scale.y * 0.5f;
    instance_data insdata;
    obj->world_to_instance_data(s * r * t, insdata);
    obj->data.push_back(insdata);
  }
  obj->size = sizeof(instance_data) * obj->data.size();
  renderer::draw2d(obj);
  *this->texture_handle.is_need_update = false;
}
void draw2d::add(const vector2 &position, const float &rotation,
                 const vector2 &scale) {
  this->worlds.push_back({position, rotation, scale});
}
void draw2d::at(const int &index, const vector2 &position,
                const float &rotation, const vector2 &scale) {
  this->worlds[index] = {position, rotation, scale};
}
void draw2d::clear() { this->worlds.clear(); }
void drawui::draw() {
  matrix4 t = matrix4::identity;
  t.mat[3][0] = position.x;
  t.mat[3][1] = position.y;
  quaternion q(vector3::neg_unit_z, rotation);
  matrix4 r = matrix4::create_from_quaternion(q);
  matrix4 s = matrix4::identity;
  s.mat[0][0] = scale.x * 0.5f;
  s.mat[1][1] = scale.y * 0.5f;
  obj->param.world = s * r * t;
  obj->binding_texture = this->texture_handle;
  matrix4 viewproj = matrix4::identity;

  auto screen_size = scene::size();
  viewproj.mat[0][0] = 2.f / screen_size.x;
  viewproj.mat[1][1] = 2.f / screen_size.y;
  obj->param.proj = viewproj;
  obj->param.view = matrix4::identity;
  obj->vertexIndex = this->vertex_name;
  for (auto &i : worlds) {
    matrix4 t = matrix4::identity;
    t.mat[3][0] = i.position.x;
    t.mat[3][1] = i.position.y;
    quaternion q(vector3::neg_unit_z, i.rotation);
    matrix4 r = matrix4::create_from_quaternion(q);
    matrix4 s = matrix4::identity;
    s.mat[0][0] = i.scale.x * 0.5f;
    s.mat[1][1] = i.scale.y * 0.5f;
    instance_data insdata;
    obj->world_to_instance_data(s * r * t, insdata);
    obj->data.push_back(insdata);
  }
  obj->size = sizeof(instance_data) * obj->data.size();
  renderer::drawui(obj);
  *this->texture_handle.is_need_update = false;
}
void drawui::add(const vector2 &position, const float &rotation,
                 const vector2 &scale) {
  this->worlds.push_back({position, rotation, scale});
}
void drawui::at(const int &index, const vector2 &position,
                const float &rotation, const vector2 &scale) {
  this->worlds[index] = {position, rotation, scale};
}
void drawui::clear() { this->worlds.clear(); }
void draw3d::draw() {
  obj->binding_texture = this->texture_handle;
  matrix4 t = matrix4::create_translation(position);
  matrix4 r = matrix4::create_from_quaternion(quaternion::from_euler(rotation));
  matrix4 s = matrix4::create_scale(scale);
  obj->param.world = s * r * t;
  obj->param.proj = scene::main_camera().projection();
  obj->param.view = scene::main_camera().view();
  obj->vertexIndex = this->vertex_name;
  for (auto &i : worlds) {
    matrix4 t = matrix4::create_translation(i.position);
    matrix4 r =
        matrix4::create_from_quaternion(quaternion::from_euler(i.rotation));
    matrix4 s = matrix4::create_scale(i.scale);
    instance_data insdata;
    obj->world_to_instance_data(s * r * t, insdata);
    obj->data.push_back(insdata);
  }
  obj->size = sizeof(instance_data) * obj->data.size();
  renderer::draw3d(obj);
  *this->texture_handle.is_need_update = false;
}
void drawui::user_data_at(int index, float value) {
  obj->param.user.mat.m16[index] = value;
}
void draw2d::user_data_at(int index, float value) {
  obj->param.user.mat.m16[index] = value;
}
void draw3d::user_data_at(int index, float value) {
  obj->param.user.mat.m16[index] = value;
}
void draw3d::add(const vector3 &position, const vector3 &rotation,
                 const vector3 &scale) {
  this->worlds.push_back({position, rotation, scale});
}
void draw3d::at(const int &index, const vector3 &position,
                const vector3 &rotation, const vector3 &scale) {
  this->worlds[index] = {position, rotation, scale};
}
void draw3d::clear() {
  this->worlds.clear();
  this->obj->data.clear();
}
} // namespace sinen
