// internal
#include <graphics/camera/camera.hpp>
#include <graphics/drawable/drawable.hpp>
#include <graphics/graphics.hpp>
#include <platform/window/window.hpp>

#include <SDL3/SDL_events.h>
namespace sinen {
Draw2D::Draw2D()
    : position(Vec2(0.f, 0.f)), rotation(0.0f), scale(Vec2(1.f, 1.f)) {
  this->model = Graphics::sprite;
}
Draw2D::Draw2D(const Texture &texture)
    : position(Vec2(0.f, 0.f)), rotation(0.0f), scale(Vec2(1.f, 1.f)) {
  this->model = Graphics::sprite;
}
Draw3D::Draw3D()
    : position(Vec3(0.f, 0.f, 0.f)), rotation(Vec3(0.f, 0.f, 0.f)),
      scale(Vec3(1.f, 1.f, 1.f)) {
  this->model = Graphics::box;
}
Draw3D::Draw3D(const Texture &texture)
    : position(Vec3(0.f, 0.f, 0.f)), rotation(Vec3(0.f, 0.f, 0.f)),
      scale(Vec3(1.f, 1.f, 1.f)) {
  this->model = Graphics::box;
}
void Draw2D::add(const Vec2 &position, const float &rotation,
                 const Vec2 &scale) {
  this->worlds.push_back({position, rotation, scale});
}
void Draw2D::at(const int &index, const Vec2 &position, const float &rotation,
                const Vec2 &scale) {
  this->worlds[index] = {position, rotation, scale};
}
void Draw2D::clear() { this->worlds.clear(); }
void Draw3D::add(const Vec3 &position, const Vec3 &rotation,
                 const Vec3 &scale) {
  Transform transform;
  transform.position = position;
  transform.rotation = rotation;
  transform.scale = scale;
  this->worlds.push_back(transform);
}
void Draw3D::at(const int &index, const Vec3 &position, const Vec3 &rotation,
                const Vec3 &scale) {
  Transform transform;
  transform.setPosition(position);
  transform.setRotation(rotation);
  transform.setScale(scale);
  this->worlds[index] = transform;
}
void Draw3D::clear() { this->worlds.clear(); }
const Model &Draw3D::getModel() const { return this->model; }
} // namespace sinen
