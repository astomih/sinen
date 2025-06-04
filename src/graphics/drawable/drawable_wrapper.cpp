// internal
#include "../../asset/model/model_data.hpp"
#include "../../graphics/graphics_system.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include <SDL3/SDL_events.h>
#include <cstring>
#include <graphics/drawable/drawable_wrapper.hpp>
#include <graphics/graphics.hpp>
#include <logic/camera/camera.hpp>
#include <logic/scene/scene.hpp>
#include <platform/window/window.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"
#include <glm/mat4x4.hpp>

namespace sinen {
Draw2D::Draw2D()
    : position(glm::vec2(0.f, 0.f)), rotation(0.0f),
      scale(glm::vec2(1.f, 1.f)) {
  obj = std::make_shared<Drawable>();
}
Draw3D::Draw3D()
    : position(glm::vec3(0.f, 0.f, 0.f)), rotation(glm::vec3(0.f, 0.f, 0.f)),
      scale(glm::vec3(1.f, 1.f, 1.f)) {
  obj = std::make_shared<Drawable>();
}
void Draw2D::draw() {
  auto ratio = glm::vec2(Window::size().x / Scene::size().x,
                         Window::size().y / Scene::size().y);

  auto t =
      glm::translate(glm::mat4(1.0f), glm::vec3(position.x * ratio.x,
                                                position.y * ratio.y, 0.0f));
  auto quaternion =
      glm::angleAxis(glm::radians(rotation), glm::vec3(0.0f, 0.0f, -1.0f));
  auto r = glm::toMat4(quaternion);

  auto s = glm::scale(glm::mat4(1.0f),
                      glm::vec3(scale.x * 0.5f, scale.y * 0.5f, 1.0f));

  obj->param.world = t * r * s;
  obj->material = this->material;
  auto viewproj = glm::mat4(1.0f);

  auto screen_size = Scene::size();
  viewproj[0][0] = 2.f / Window::size().x;
  viewproj[1][1] = 2.f / Window::size().y;
  obj->param.proj = viewproj;
  obj->param.view = glm::mat4(1.f);
  if (GetModelData(this->model.data)->vertexBuffer == nullptr) {
    obj->model = GraphicsSystem::sprite;
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

    InstanceData insdata;
    obj->world_to_instance_data(world, insdata);
    obj->data.push_back(insdata);
  }
  Graphics::draw2d(obj);
}
void Draw2D::add(const glm::vec2 &position, const float &rotation,
                 const glm::vec2 &scale) {
  this->worlds.push_back({position, rotation, scale});
}
void Draw2D::at(const int &index, const glm::vec2 &position,
                const float &rotation, const glm::vec2 &scale) {
  this->worlds[index] = {position, rotation, scale};
}
void Draw2D::clear() { this->worlds.clear(); }
void Draw3D::draw() {
  obj->material = this->material;
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
  obj->param.world = world;
  obj->param.proj = Scene::camera().projection();
  obj->param.view = Scene::camera().view();
  if (GetModelData(this->model.data)->vertexBuffer == nullptr) {
    obj->model = GraphicsSystem::box;
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

    obj->world_to_instance_data(world, insdata);
    obj->data.push_back(insdata);
  }
  Graphics::draw3d(obj);
}
void Draw2D::user_data_at(int index, float value) {}
void Draw3D::user_data_at(int index, float value) {}
void Draw3D::add(const glm::vec3 &position, const glm::vec3 &rotation,
                 const glm::vec3 &scale) {
  this->worlds.push_back({position, rotation, scale});
}
void Draw3D::at(const int &index, const glm::vec3 &position,
                const glm::vec3 &rotation, const glm::vec3 &scale) {
  this->worlds[index] = {position, rotation, scale};
}
void Draw3D::clear() {
  this->worlds.clear();
  this->obj->data.clear();
}
} // namespace sinen
