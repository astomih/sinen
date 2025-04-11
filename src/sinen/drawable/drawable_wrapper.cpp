// internal
#include "../model/model_data.hpp"
#include "../render/render_system.hpp"
#include <camera/camera.hpp>
#include <cstring>
#include <drawable/drawable_wrapper.hpp>
#include <render/renderer.hpp>
#include <scene/scene.hpp>
#include <window/window.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"
#include <glm/mat4x4.hpp>

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
  auto ratio = Vector2(Window::size().x / Scene::size().x,
                       Window::size().y / Scene::size().y);

  auto t =
      glm::translate(glm::mat4(1.0f), glm::vec3(position.x * ratio.x,
                                                position.y * ratio.y, 0.0f));
  auto quaternion =
      glm::angleAxis(glm::radians(rotation), glm::vec3(0.0f, 0.0f, -1.0f));
  auto r = glm::toMat4(quaternion);

  auto s = glm::scale(glm::mat4(1.0f),
                      glm::vec3(scale.x * 0.5f, scale.y * 0.5f, 1.0f));

  auto world = t * r * s;
  memcpy(obj->param.world.get(), &world[0][0], sizeof(float) * 16);
  obj->binding_texture = this->texture_handle;
  auto viewproj = glm::mat4(1.0f);

  auto screen_size = Scene::size();
  viewproj[0][0] = 2.f / Window::size().x;
  viewproj[1][1] = 2.f / Window::size().y;
  memcpy(obj->param.proj.get(), &viewproj[0][0], sizeof(float) * 16);
  obj->param.view = matrix4::identity;
  if (GetModelData(this->model.data)->vertexBuffer == nullptr) {
    obj->model = RendererImpl::sprite;
  } else
    obj->model = this->model;
  for (auto &i : worlds) {
    auto t = glm::translate(
        glm::mat4(1.0f),
        glm::vec3(i.position.x * ratio.x, i.position.y * ratio.y, 0.0f));
    auto quaternion =
        glm::angleAxis(glm::radians(i.rotation), glm::vec3(0.0f, 0.0f, -1.0f));
    auto r = glm::toMat4(quaternion);
    auto s = glm::scale(glm::mat4(1.0f),
                        glm::vec3(i.scale.x * 0.5f, i.scale.y * 0.5f, 1.0f));

    auto world = t * r * s;
    matrix4 mat = matrix4::identity;
    memcpy(&mat, &world[0][0], sizeof(float) * 16);

    InstanceData insdata;
    obj->world_to_instance_data(mat, insdata);
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
  auto t = glm::translate(glm::mat4(1.0f),
                          glm::vec3(position.x, position.y, position.z));
  auto rotationX =
      glm::angleAxis(glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
  auto rotationY =
      glm::angleAxis(glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
  auto rotationZ =
      glm::angleAxis(glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
  auto r = glm::toMat4(rotationX * rotationY * rotationZ);

  auto s = glm::scale(glm::mat4(1.0f), glm::vec3(scale.x, scale.y, scale.z));

  auto world = t * r * s;
  memcpy(obj->param.world.get(), &world[0][0], sizeof(float) * 16);
  obj->param.proj = Scene::main_camera().projection();
  obj->param.view = Scene::main_camera().view();
  if (GetModelData(this->model.data)->vertexBuffer == nullptr) {
    obj->model = RendererImpl::box;
  } else
    obj->model = this->model;
  for (auto &i : worlds) {
    InstanceData insdata;
    auto t = glm::translate(
        glm::mat4(1.0f), glm::vec3(i.position.x, i.position.y, i.position.z));
    auto rotationX =
        glm::angleAxis(glm::radians(i.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    auto rotationY =
        glm::angleAxis(glm::radians(i.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    auto rotationZ =
        glm::angleAxis(glm::radians(i.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    auto r = glm::toMat4(rotationX * rotationY * rotationZ);

    auto s =
        glm::scale(glm::mat4(1.0f), glm::vec3(i.scale.x, i.scale.y, i.scale.z));

    auto world = t * r * s;
    matrix4 mat = matrix4::identity;
    memcpy(&mat, &world[0][0], sizeof(float) * 16);
    obj->world_to_instance_data(mat, insdata);
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
