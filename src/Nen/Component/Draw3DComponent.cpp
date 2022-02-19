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
  auto view = mOwner.GetScene().GetRenderer()->GetViewMatrix();

  sprite->param.view = view;
  sprite->param.world = world;
}

draw_3d_component::~draw_3d_component() {
  mOwner.GetScene().GetRenderer()->RemoveDrawObject3D(sprite);
}

void draw_3d_component::Create(std::shared_ptr<texture> _texture,
                               std::string_view shape) {
  if (_texture)
    mTexture = _texture;
  else {
    auto tex = std::make_shared<texture>();
    tex->CreateFromColor(palette::White, "none");
    Create(tex);
  }
  mTexWidth = mTexture->GetWidth();
  mTexHeight = mTexture->GetHeight();
  auto renderer = mOwner.GetScene().GetRenderer();
  auto scaleOwner = mOwner.GetScale();
  auto view = mOwner.GetScene().GetRenderer()->GetViewMatrix();
  auto proj = mOwner.GetScene().GetRenderer()->GetProjectionMatrix();
  sprite = std::make_shared<draw_object>();
  sprite->textureIndex = mTexture->id;
  sprite->vertexIndex = shape.data();
  mOwner.ComputeWorldTransform();
  sprite->param.world = mOwner.GetWorldTransform();
  sprite->param.proj = proj;
  sprite->param.view = view;
}
void draw_3d_component::Register() {
  if (sprite)
    mOwner.GetScene().GetRenderer()->AddDrawObject3D(sprite, mTexture);
}
} // namespace nen
