// internal
#include "../model/model_data.hpp"
#include "../render/render_system.hpp"
#include <camera/camera.hpp>
#include <drawable/drawable_wrapper.hpp>
#include <render/renderer.hpp>
#include <scene/scene.hpp>
#include <window/window.hpp>

namespace sinen {
Draw2D::Draw2D()
    : position(Vector2(0.f, 0.f)), rotation(0.0f), scale(Vector2(1.f, 1.f)) {
  obj = std::make_shared<Drawable>();
}
Draw2D::Draw2D(Texture texture_handle)
    : position(Vector2(0.f, 0.f)), rotation(0.0f), scale(Vector2(1.f, 1.f)),
      texture_handle(texture_handle) {
  obj = std::make_shared<Drawable>();
}
Draw3D::Draw3D()
    : position(Vector3(0.f, 0.f, 0.f)), rotation(Vector3(0.f, 0.f, 0.f)),
      scale(Vector3(1.f, 1.f, 1.f)) {
  obj = std::make_shared<Drawable>();
}
Draw3D::Draw3D(Texture texture_handle)
    : position(Vector3(0.f, 0.f, 0.f)), rotation(Vector3(0.f, 0.f, 0.f)),
      scale(Vector3(1.f, 1.f, 1.f)), texture_handle(texture_handle) {
  obj = std::make_shared<Drawable>();
}
void Draw2D::draw() {
  matrix4 t = matrix4::identity;
  t.mat[3][0] = position.x;
  t.mat[3][1] = position.y;
  Quaternion q(Vector3::neg_unit_z, rotation);
  matrix4 r = matrix4::create_from_quaternion(q);
  matrix4 s = matrix4::identity;
  s.mat[0][0] = scale.x * 0.5f;
  s.mat[1][1] = scale.y * 0.5f;
  obj->param.world = s * r * t;
  obj->binding_texture = this->texture_handle;
  matrix4 viewproj = matrix4::identity;

  auto screen_size = Scene::size();
  viewproj.mat[0][0] = 2.f / screen_size.x;
  viewproj.mat[1][1] = 2.f / screen_size.y;
  obj->param.proj = viewproj;
  obj->param.view = matrix4::identity;
  if (GetModelData(this->model.data)->vertexBuffer == nullptr) {
    obj->model = RendererImpl::sprite;
  } else
    obj->model = this->model;
  for (auto &i : worlds) {
    matrix4 t = matrix4::identity;
    t.mat[3][0] = i.position.x;
    t.mat[3][1] = i.position.y;
    Quaternion q(Vector3::neg_unit_z, i.rotation);
    matrix4 r = matrix4::create_from_quaternion(q);
    matrix4 s = matrix4::identity;
    s.mat[0][0] = i.scale.x * 0.5f;
    s.mat[1][1] = i.scale.y * 0.5f;
    InstanceData insdata;
    obj->world_to_instance_data(s * r * t, insdata);
    obj->data.push_back(insdata);
  }
  Renderer::draw2d(obj);
}
void Draw2D::add(const Vector2 &position, const float &rotation,
                 const Vector2 &scale) {
  this->worlds.push_back({position, rotation, scale});
}
void Draw2D::at(const int &index, const Vector2 &position,
                const float &rotation, const Vector2 &scale) {
  this->worlds[index] = {position, rotation, scale};
}
void Draw2D::clear() { this->worlds.clear(); }
void Draw3D::draw() {
  obj->binding_texture = this->texture_handle;
  matrix4 t = matrix4::create_translation(position);
  matrix4 r = matrix4::create_from_quaternion(Quaternion::from_euler(rotation));
  matrix4 s = matrix4::create_scale(scale);
  obj->param.world = s * r * t;
  obj->param.proj = Scene::main_camera().projection();
  obj->param.view = Scene::main_camera().view();
  if (GetModelData(this->model.data)->vertexBuffer == nullptr) {
    obj->model = RendererImpl::box;
  } else
    obj->model = this->model;
  for (auto &i : worlds) {
    matrix4 t = matrix4::create_translation(i.position);
    matrix4 r =
        matrix4::create_from_quaternion(Quaternion::from_euler(i.rotation));
    matrix4 s = matrix4::create_scale(i.scale);
    InstanceData insdata;
    obj->world_to_instance_data(s * r * t, insdata);
    obj->data.push_back(insdata);
  }
  Renderer::draw3d(obj);
}
void Draw2D::user_data_at(int index, float value) {
  obj->param.user.mat.m16[index] = value;
}
void Draw3D::user_data_at(int index, float value) {
  obj->param.user.mat.m16[index] = value;
}
void Draw3D::add(const Vector3 &position, const Vector3 &rotation,
                 const Vector3 &scale) {
  this->worlds.push_back({position, rotation, scale});
}
void Draw3D::at(const int &index, const Vector3 &position,
                const Vector3 &rotation, const Vector3 &scale) {
  this->worlds[index] = {position, rotation, scale};
}
void Draw3D::clear() {
  this->worlds.clear();
  this->obj->data.clear();
}
} // namespace sinen
