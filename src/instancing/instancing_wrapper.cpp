#include <instancing/instancing.hpp>
#include <instancing/instancing_wrapper.hpp>

#include <camera/camera.hpp>
#include <manager/manager.hpp>
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
  obj->texture_handle = this->texture_handle.handle;
  obj->vertexIndex = this->vertex_name;
  obj->texture_handle = texture_handle.handle;
  matrix4 viewproj = matrix4::Identity;

  auto windowsize = get_window().Size();
  viewproj.mat[0][0] = 2.f / windowsize.x;
  viewproj.mat[1][1] = 2.f / windowsize.y;
  obj->param.proj = viewproj;
  obj->param.view = matrix4::Identity;
  obj->vertexIndex = this->vertex_name;
  _instancing.object = obj;
  _instancing.type = object_type::_2D;
  for (int i = 0; i < this->position.size(); i++) {
    matrix4 t = matrix4::Identity;
    t.mat[3][0] = position[i].x;
    t.mat[3][1] = position[i].y;
    quaternion q(vector3::neg_unit_z, rotation[i]);
    matrix4 r = matrix4::CreateFromQuaternion(q);
    matrix4 s = matrix4::Identity;
    s.mat[0][0] = scale[i].x * 0.5f;
    s.mat[1][1] = scale[i].y * 0.5f;
    instance_data insdata;
    _instancing.world_to_instance_data(s * r * t, insdata);
    _instancing.data.push_back(insdata);
  }
  _instancing.size = sizeof(instance_data) * _instancing.data.size();

  get_renderer().add_instancing(_instancing);
}
void draw2d_instancing::add(const vector2 &position, const float &rotation,
                            const vector2 &scale) {
  this->position.push_back(position);
  this->rotation.push_back(rotation);
  this->scale.push_back(scale);
}

draw3d_instancing::draw3d_instancing(texture texture_handle)
    : texture_handle(texture_handle) {
  vertex_name = "SPRITE";
}
void draw3d_instancing::draw() {
  instancing _instancing;
  auto obj = std::make_shared<draw_object>();
  obj->texture_handle = this->texture_handle.handle;
  obj->vertexIndex = this->vertex_name;
  obj->param.proj = get_camera().projection;
  obj->param.view = get_camera().view;
  _instancing.object = obj;
  _instancing.type = object_type::_3D;
  for (int i = 0; i < this->position.size(); i++) {
    matrix4 t = matrix4::Identity;
    t.mat[3][0] = position[i].x;
    t.mat[3][1] = position[i].y;
    t.mat[3][2] = position[i].z;
    quaternion q(vector3::neg_unit_z, rotation[i].z);
    q = quaternion::Concatenate(q, quaternion(vector3::unit_y, rotation[i].y));
    q = quaternion::Concatenate(q, quaternion(vector3::unit_x, rotation[i].x));
    matrix4 r = matrix4::CreateFromQuaternion(q);
    matrix4 s = matrix4::Identity;
    s.mat[0][0] = scale[i].x;
    s.mat[1][1] = scale[i].y;
    s.mat[2][2] = scale[i].z;
    instance_data insdata;
    _instancing.world_to_instance_data(s * r * t, insdata);
    _instancing.data.push_back(insdata);
  }
  _instancing.size = sizeof(instance_data) * _instancing.data.size();
  get_renderer().add_instancing(_instancing);
}
void draw3d_instancing::add(const vector3 &position, const vector3 &rotation,
                            const vector3 &scale) {
  this->position.push_back(position);
  this->rotation.push_back(rotation);
  this->scale.push_back(scale);
}
} // namespace nen