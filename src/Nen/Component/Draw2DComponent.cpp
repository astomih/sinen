#include <Nen.hpp>
#include <iostream>
#include <SDL_image.h>
namespace nen
{
	constexpr int matrixSize = sizeof(float) * 16;
	Draw2DComponent::Draw2DComponent(Actor &owner, const int drawOrder, Texture tex)
		: Component(owner), mDrawOrder(drawOrder), mTexture(nullptr)
	{
	}

	void Draw2DComponent::Update(float deltaTime)
	{
		if (!mTexture)
		{
			auto tex = std::make_shared<Texture>();
			tex->Load("Assets/Default.png");
			Create(tex);
		}

		auto w = mOwner.GetWorldTransform();
		Matrix4 s = Matrix4::Identity;
		s.mat[0][0] = mTexture->GetWidth();
		s.mat[1][1] = mTexture->GetHeight();
		auto world = s * w;
		world.mat[3][0] *= 2.f;
		world.mat[3][1] *= 2.f;
		sprite->param.world = world;
	}

	Draw2DComponent::~Draw2DComponent()
	{
		mOwner.GetScene().GetRenderer()->RemoveDrawObject2D(sprite);
	}

	void Draw2DComponent::Create(std::shared_ptr<Texture> texture, const float scale, std::string_view shape)
	{
		if (texture)
			mTexture = texture;
		else
		{
			auto tex = std::make_shared<Texture>();
			tex->Load("Assets/Texture/Default.png");
			mTexture = tex;
		}
		mTexWidth = mTexture->GetWidth();
		mTexHeight = mTexture->GetHeight();
		auto renderer = mOwner.GetScene().GetRenderer();
		auto api = renderer->GetGraphicsAPI();
		auto scaleOwner = mOwner.GetScale();
		Matrix4 viewproj = Matrix4::Identity;
		viewproj.mat[0][0] = 1.f / Window::Size.x;
		viewproj.mat[1][1] = 1.f / Window::Size.y;
		sprite = std::make_shared<DrawObject>();
		sprite->drawOrder = mDrawOrder;
		sprite->textureIndex = mTexture->id;
		sprite->vertexIndex = shape.data();

		sprite->param.world = mOwner.GetWorldTransform();
		sprite->param.proj = viewproj;
		sprite->param.view = Matrix4::Identity;
		mOwner.GetScene().GetRenderer()->AddDrawObject2D(sprite, texture);
	}
}