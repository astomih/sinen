#include "Color/Color.hpp"
#include <Nen.hpp>
#include <SDL_image.h>
#include <iostream>
#include <memory>

namespace nen {
draw_3d_component::draw_3d_component(base_actor &owner, int drawOrder)
    : base_component(owner), mOwner(owner) {}

void draw_3d_component::Update(float deltaTime) {
  auto world = mOwner.GetWorldTransform();
  auto view = mOwner.GetScene().GetRenderer().GetViewMatrix();

  sprite->param.view = view;
  sprite->param.world = world;
}

draw_3d_component::~draw_3d_component() {
  mOwner.GetScene().GetRenderer().RemoveDrawObject3D(sprite);
}

void draw_3d_component::Create(handle_t handle, std::string_view shape) {
  auto &mTexture = mOwner.GetScene().get_texture().get_texture(handle);
  mTexWidth = mTexture.GetWidth();
  mTexHeight = mTexture.GetHeight();
  auto view = mOwner.GetScene().GetRenderer().GetViewMatrix();
  auto proj = mOwner.GetScene().GetRenderer().GetProjectionMatrix();
  sprite = std::make_shared<draw_object>();
  sprite->texture_handle = handle;
  sprite->textureIndex = mTexture.id;
  sprite->vertexIndex = shape.data();
  mOwner.ComputeWorldTransform();
  sprite->param.world = mOwner.GetWorldTransform();
  sprite->param.proj = proj;
  sprite->param.view = view;
}
void draw_3d_component::Register() {
  if (sprite)
    mOwner.GetScene().GetRenderer().AddDrawObject3D(sprite);
}
} // namespace nen
