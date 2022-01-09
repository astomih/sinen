#include <Nen.hpp>
#include <SDL_image.h>
#include <iostream>

namespace nen {
const int matrixSize = sizeof(float) * 16;
draw_2d_component::draw_2d_component(base_actor &owner, const int drawOrder)
    : base_component(owner), mDrawOrder(drawOrder), mTexture(nullptr) {}

void draw_2d_component::Update(float deltaTime) {
  auto w = mOwner.GetWorldTransform();
  matrix4 s = matrix4::Identity;
  s.mat[0][0] = static_cast<float>(mTexture->GetWidth());
  s.mat[1][1] = static_cast<float>(mTexture->GetHeight());
  auto world = s * w;
  world.mat[3][0] *= 2.f;
  world.mat[3][1] *= 2.f;
  sprite->param.world = world;
}

draw_2d_component::~draw_2d_component() {
  mOwner.GetScene().GetRenderer()->RemoveDrawObject2D(sprite);
}

void draw_2d_component::Create(std::shared_ptr<texture> _texture,
                               const float scale, std::string_view shape) {
  mTexture = _texture;
  mTexWidth = mTexture->GetWidth();
  mTexHeight = mTexture->GetHeight();
  auto renderer = mOwner.GetScene().GetRenderer();
  matrix4 viewproj = matrix4::Identity;
  auto windowsize = mOwner.GetScene().GetRenderer()->GetWindow()->Size();
  viewproj.mat[0][0] = 1.f / windowsize.x;
  viewproj.mat[1][1] = 1.f / windowsize.y;
  sprite = std::make_shared<draw_object>();
  sprite->drawOrder = mDrawOrder;
  sprite->textureIndex = mTexture->id;
  sprite->vertexIndex = shape.data();

  mOwner.ComputeWorldTransform();
  sprite->param.world = mOwner.GetWorldTransform();
  sprite->param.proj = viewproj;
  sprite->param.view = matrix4::Identity;
}
void draw_2d_component::Register() {
  if (sprite && mTexture)
    mOwner.GetScene().GetRenderer()->AddDrawObject2D(sprite, mTexture);
}
} // namespace nen