#include <instancing/instancing.hpp>
#include <instancing/instancing_wrapper.hpp>

#include "../manager/get_system.hpp"
#include "../render/render_system.hpp"
#include "../window/window_system.hpp"
#include <camera/camera.hpp>
#include <render/renderer.hpp>
#include <window/window.hpp>

namespace nen {
draw2d_instancing::draw2d_instancing(texture texture_handle)
    : texture_handle(texture_handle) {
  vertex_name = "SPRITE";
}

void draw2d_instancing::draw() {

  instancing _instancing;
  auto obj = std::make_shared<draw_object>();
  obj->texture_handle = this->texture_handle;
  obj->vertexIndex = this->vertex_name;
  matrix4 viewproj = matrix4::identity;

  auto windowsize = get_window().size;
  viewproj.mat[0][0] = 2.f / windowsize.x;
  viewproj.mat[1][1] = 2.f / windowsize.y;
  obj->param.proj = viewproj;
  obj->param.view = matrix4::identity;
  obj->vertexIndex = this->vertex_name;
  _instancing.object = obj;
  _instancing.type = object_type::_2D;
  for (int i = 0; i < this->position.size(); i++) {
    matrix4 t = matrix4::identity;
    t.mat[3][0] = position[i].x;
    t.mat[3][1] = position[i].y;
    quaternion q(vector3::neg_unit_z, rotation[i]);
    matrix4 r = matrix4::create_from_quaternion(q);
    matrix4 s = matrix4::identity;
    s.mat[0][0] = scale[i].x * 0.5f;
    s.mat[1][1] = scale[i].y * 0.5f;
    instance_data insdata;
    _instancing.world_to_instance_data(s * r * t, insdata);
    _instancing.data.push_back(insdata);
  }
  _instancing.size = sizeof(instance_data) * _instancing.data.size();

  get_renderer().add_instancing(_instancing);
  *this->texture_handle.is_need_update = false;
}
void draw2d_instancing::add(const vector2 &position, const float &rotation,
                            const vector2 &scale) {
  this->position.push_back(position);
  this->rotation.push_back(rotation);
  this->scale.push_back(scale);
}

void draw2d_instancing::clear() {
  this->position.clear();
  this->rotation.clear();
  this->scale.clear();
}

draw3d_instancing::draw3d_instancing(texture texture_handle)
    : texture_handle(texture_handle) {
  vertex_name = "SPRITE";
}
void draw3d_instancing::draw() {
  instancing _instancing;
  auto obj = std::make_shared<draw_object>();
  obj->texture_handle = this->texture_handle;
  obj->vertexIndex = this->vertex_name;
  obj->param.proj = get_camera().projection;
  obj->param.view = get_camera().view;
  obj->is_draw_depth = this->is_draw_depth;
  _instancing.object = obj;
  _instancing.type = object_type::_3D;
  for (int i = 0; i < this->position.size(); i++) {
    matrix4 t = matrix4::identity;
    t.mat[3][0] = position[i].x;
    t.mat[3][1] = position[i].y;
    t.mat[3][2] = position[i].z;
    quaternion q(vector3::neg_unit_z, rotation[i].z);
    q = quaternion::concatenate(q, quaternion(vector3::unit_y, rotation[i].y));
    q = quaternion::concatenate(q, quaternion(vector3::unit_x, rotation[i].x));
    matrix4 r = matrix4::create_from_quaternion(q);
    matrix4 s = matrix4::identity;
    s.mat[0][0] = scale[i].x;
    s.mat[1][1] = scale[i].y;
    s.mat[2][2] = scale[i].z;
    instance_data insdata;
    _instancing.world_to_instance_data(s * r * t, insdata);
    _instancing.data.push_back(insdata);
  }
  _instancing.size = sizeof(instance_data) * _instancing.data.size();
  get_renderer().add_instancing(_instancing);
  *this->texture_handle.is_need_update = false;
}
void draw3d_instancing::add(const vector3 &position, const vector3 &rotation,
                            const vector3 &scale) {
  this->position.push_back(position);
  this->rotation.push_back(rotation);
  this->scale.push_back(scale);
}
void draw3d_instancing::clear() {
  this->position.clear();
  this->rotation.clear();
  this->scale.clear();
}
} // namespace nen