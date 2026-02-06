// internal
#include <graphics/camera/camera3d.hpp>
#include <graphics/drawable/drawable.hpp>
#include <graphics/graphics.hpp>
#include <platform/window/window.hpp>

#include <SDL3/SDL_events.h>
namespace sinen {
Draw2D::Draw2D()
    : position(Vec2(0.f, 0.f)), rotation(0.0f), scale(Vec2(1.f, 1.f)) {}
Draw2D::Draw2D(const Texture &texture)
    : position(Vec2(0.f, 0.f)), rotation(0.0f), scale(Vec2(1.f, 1.f)) {}
Draw3D::Draw3D()
    : position(Vec3(0.f, 0.f, 0.f)), rotation(Vec3(0.f, 0.f, 0.f)),
      scale(Vec3(1.f, 1.f, 1.f)) {}
Draw3D::Draw3D(const Texture &texture)
    : position(Vec3(0.f, 0.f, 0.f)), rotation(Vec3(0.f, 0.f, 0.f)),
      scale(Vec3(1.f, 1.f, 1.f)) {}
const Model &Draw3D::getModel() const { return this->model; }
} // namespace sinen
