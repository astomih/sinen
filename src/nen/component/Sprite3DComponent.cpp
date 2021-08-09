#include <nen.hpp>
#include <iostream>
#include <SDL_image.h>
namespace nen
{
	Sprite3DComponent::Sprite3DComponent(Actor &owner, const int drawOrder, Texture tex)
		: Component(owner), mOwner(owner), mDrawOrder(drawOrder)
	{
	}

	void Sprite3DComponent::Update(float deltaTime)
	{
		auto world = mOwner.GetWorldTransform();
		auto view = mOwner.GetScene().GetRenderer()->GetViewMatrix();
		sprite->param.view = view;
		sprite->param.world = world;
	}

	Sprite3DComponent::~Sprite3DComponent()
	{
		mOwner.GetScene().GetRenderer()->RemoveSprite3D(sprite);
	}

	void Sprite3DComponent::Draw(Shader *shader)
	{
	}

	void Sprite3DComponent::Create(std::shared_ptr<Texture> texture, std::string_view shape)
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
		const auto yScale = Math::Cot(Math::ToRadians(90.f) / 2.0f);
		const auto xScale = yScale / (Window::Size.x / Window::Size.y);
		const float zfar = 10000.f;
		const float znear = 0.01f;
		float temp[4][4] =
			{
				{xScale, 0.0f, 0.0f, 0.0f},
				{0.0f, yScale, 0.0f, 0.0f},
				{0.0f, 0.0f, zfar/ (znear - zfar), -1.0f},
				{0.0f, 0.0f, znear * zfar / (znear - zfar), 0.0f}};
		Matrix4 proj(temp);
		sprite = std::make_shared<Sprite>();
		sprite->textureIndex = mTexture->id;
		sprite->vertexIndex = shape.data();
		mOwner.Move(0, 0, 0);
		mOwner.ComputeWorldTransform();
		sprite->param.world = mOwner.GetWorldTransform();
		sprite->param.proj = proj;
		sprite->param.view = view;

		mOwner.GetScene().GetRenderer()->AddSprite3D(sprite, texture);
	}
	void Sprite3DComponent::SetTrimmingStartPos(const Vector2i &pos)
	{
		sprite->isChangeBuffer = true;
		mOwner.GetScene().GetRenderer()->ChangeBufferSprite(sprite, TextureType::Image3D);
		sprite->trimStart.x = (float)pos.x / (float)mTexWidth;
		sprite->trimStart.y = (float)pos.y / (float)mTexHeight;
	}
	void Sprite3DComponent::SetTrimmingEndPos(const Vector2i &pos)
	{
		sprite->isChangeBuffer = true;
		mOwner.GetScene().GetRenderer()->ChangeBufferSprite(sprite, TextureType::Image3D);
		sprite->trimEnd.x = (float)pos.x / (float)mTexWidth;
		sprite->trimEnd.y = (float)pos.y / (float)mTexHeight;
	}
}
