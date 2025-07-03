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
Draw2D::Draw2D(const Texture &texture)
    : position(glm::vec2(0.f, 0.f)), rotation(0.0f),
      scale(glm::vec2(1.f, 1.f)) {
  this->material.SetTexture(texture);
  obj = std::make_shared<Drawable>();
}
Draw3D::Draw3D()
    : position(glm::vec3(0.f, 0.f, 0.f)), rotation(glm::vec3(0.f, 0.f, 0.f)),
      scale(glm::vec3(1.f, 1.f, 1.f)) {
  obj = std::make_shared<Drawable>();
}
Draw3D::Draw3D(const Texture &texture)
    : position(glm::vec3(0.f, 0.f, 0.f)), rotation(glm::vec3(0.f, 0.f, 0.f)),
      scale(glm::vec3(1.f, 1.f, 1.f)) {
  this->material.SetTexture(texture);
  obj = std::make_shared<Drawable>();
}
void Draw2D::Add(const glm::vec2 &position, const float &rotation,
                 const glm::vec2 &scale) {
  this->worlds.push_back({position, rotation, scale});
}
void Draw2D::At(const int &index, const glm::vec2 &position,
                const float &rotation, const glm::vec2 &scale) {
  this->worlds[index] = {position, rotation, scale};
}
void Draw2D::Clear() { this->worlds.clear(); }
void Draw3D::Add(const glm::vec3 &position, const glm::vec3 &rotation,
                 const glm::vec3 &scale) {
  Transform3D transform;
  transform.SetPosition(position);
  transform.SetRotation(rotation);
  transform.SetScale(scale);
  this->worlds.push_back(transform);
}
void Draw3D::At(const int &index, const glm::vec3 &position,
                const glm::vec3 &rotation, const glm::vec3 &scale) {
  Transform3D transform;
  transform.SetPosition(position);
  transform.SetRotation(rotation);
  transform.SetScale(scale);
  this->worlds[index] = transform;
}
void Draw3D::Clear() {
  this->worlds.clear();
  this->obj->data.clear();
}
} // namespace sinen
