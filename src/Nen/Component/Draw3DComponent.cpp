#include <Nen.hpp>
#include <iostream>
#include <SDL_image.h>
namespace nen
{
	Draw3DComponent::Draw3DComponent(Actor &owner, int drawOrder)
		: Component(owner), mOwner(owner)
	{
	}

	void Draw3DComponent::Update(float deltaTime)
	{
		auto world = mOwner.GetWorldTransform();
		auto view = mOwner.GetScene().GetRenderer()->GetViewMatrix();

		sprite->param.view = view;
		sprite->param.world = world;
	}

	Draw3DComponent::~Draw3DComponent()
	{
		mOwner.GetScene().GetRenderer()->RemoveDrawObject3D(sprite);
	}

	void Draw3DComponent::Create(std::shared_ptr<Texture> texture, std::string_view shape)
	{
		if (texture)
			mTexture = texture;
		else
		{
			auto tex = std::make_shared<Texture>();
			tex->Load("Assets/Default.png");
			Create(tex);
		}
		mTexWidth = mTexture->GetWidth();
		mTexHeight = mTexture->GetHeight();
		auto renderer = mOwner.GetScene().GetRenderer();
		auto scaleOwner = mOwner.GetScale();
		auto pos = mOwner.GetPosition();
		auto view = mOwner.GetScene().GetRenderer()->GetViewMatrix();
		/*
		const auto yScale = Math::Cot(Math::ToRadians(90.f) / 2.0f);
		const auto xScale = yScale / (Window::Size.x / Window::Size.y);
		const float zfar = 10000.f;
		const float znear = 0.01f;
		float temp[4][4] =
			{
				{xScale, 0.0f, 0.0f, 0.0f},
				{0.0f, yScale, 0.0f, 0.0f},
				{0.0f, 0.0f, zfar / (znear - zfar), -1.0f},
				{0.0f, 0.0f, znear * zfar / (znear - zfar), 0.0f}};
		Matrix4 proj(temp);
		*/
		auto proj = Matrix4::Perspective(Math::ToRadians(90.f), Window::Size.x / Window::Size.y, 0.01f, 10000.f);
		sprite = std::make_shared<DrawObject>();
		sprite->textureIndex = mTexture->id;
		sprite->vertexIndex = shape.data();
		mOwner.Move(0, 0, 0);
		mOwner.ComputeWorldTransform();
		sprite->param.world = mOwner.GetWorldTransform();
		sprite->param.proj = proj;
		sprite->param.view = view;

		mOwner.GetScene().GetRenderer()->AddDrawObject3D(sprite, texture);
	}
}
