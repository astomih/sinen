#include <Nen.hpp>
#include <SDL_image.h>
#include <iostream>

namespace nen {
const int matrixSize = sizeof(float) * 16;
draw_2d_component::draw_2d_component(base_actor &owner, const int drawOrder)
    : base_component(owner), mDrawOrder(drawOrder) {}

void draw_2d_component::Update(float deltaTime) {
  auto w = mOwner.GetWorldTransform();
  matrix4 s = matrix4::Identity;
  auto &mTexture =
      GetActor().GetScene().get_texture().get_texture(sprite->texture_handle);
  s.mat[0][0] = static_cast<float>(mTexture.GetWidth());
  s.mat[1][1] = static_cast<float>(mTexture.GetHeight());
  auto world = s * w;
  world.mat[3][0] *= 2.f;
  world.mat[3][1] *= 2.f;
  sprite->param.world = world;
}

draw_2d_component::~draw_2d_component() {
  mOwner.GetScene().GetRenderer().RemoveDrawObject2D(sprite);
}

void draw_2d_component::Create(handle_t handle, const float scale,
                               std::string_view shape) {

  auto &mTexture = GetActor().GetScene().get_texture().get_texture(handle);
  mTexWidth = mTexture.GetWidth();
  mTexHeight = mTexture.GetHeight();
  matrix4 viewproj = matrix4::Identity;
  auto windowsize = mOwner.GetScene().GetRenderer().GetWindow().Size();
  viewproj.mat[0][0] = 1.f / windowsize.x;
  viewproj.mat[1][1] = 1.f / windowsize.y;
  sprite = std::make_shared<draw_object>();
  sprite->texture_handle = handle;
  sprite->drawOrder = mDrawOrder;
  sprite->textureIndex = mTexture.id;
  sprite->vertexIndex = shape.data();

  mOwner.ComputeWorldTransform();
  sprite->param.world = mOwner.GetWorldTransform();
  sprite->param.proj = viewproj;
  sprite->param.view = matrix4::Identity;
  mOwner.GetScene().GetRenderer().AddDrawObject2D(sprite);
}
void draw_2d_component::Register() {
  if (sprite)
    mOwner.GetScene().GetRenderer().AddDrawObject2D(sprite);
}
} // namespace nen