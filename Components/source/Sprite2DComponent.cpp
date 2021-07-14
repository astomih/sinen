#include <Components.hpp>
#include <Actors.hpp>
#include <Engine.hpp>
#include <Scene.hpp>
#include <iostream>
#include <SDL_image.h>
namespace nen
{
	constexpr int matrixSize = sizeof(float) * 16;
	Sprite2DComponent::Sprite2DComponent(Actor &owner, const int drawOrder, Texture tex)
		: Component(owner), mDrawOrder(drawOrder), mTexture(nullptr)
	{
	}

	void Sprite2DComponent::Update(float deltaTime)
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

	Sprite2DComponent::~Sprite2DComponent()
	{
		mOwner.GetScene().GetRenderer()->RemoveSprite2D(sprite);
	}

	void Sprite2DComponent::Draw(Shader *shader)
	{
	}

	void Sprite2DComponent::Create(std::shared_ptr<Texture> texture, const float scale, std::string_view shape)
	{
		if (texture)
			mTexture = texture;
		else
		{
			auto tex = std::make_shared<Texture>();
			tex->Load("Assets/Default.png");
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
		sprite = std::make_shared<Sprite>();
		sprite->drawOrder = mDrawOrder;
		sprite->textureIndex = mTexture->id;
		sprite->vertexIndex = shape.data();

		sprite->param.world = mOwner.GetWorldTransform();
		sprite->param.proj = viewproj;
		sprite->param.view = Matrix4::Identity;
		mOwner.GetScene().GetRenderer()->AddSprite2D(sprite, texture);
	}
	void Sprite2DComponent::SetTrimmingStartPos(const Vector2i &pos)
	{
		/*
			mTextureVK->sprite->trimStart.x = (float)pos.x / (float)mTexWidth;
			mTextureVK->sprite->trimStart.y = (float)pos.y / (float)mTexHeight;
			if (mTextureVK->sprite->isChangeBuffer == false)
			{
				mTextureVK->buffer = mOwner.GetScene()->GetRenderer()->GetVK().CreateBuffer(
					sizeof(Vertex) * 4, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
				mTextureVK->sprite->isChangeBuffer = true;
			}
			*/
		sprite->isChangeBuffer = true;
		mOwner.GetScene().GetRenderer()->ChangeBufferSprite(sprite);
		sprite->trimStart.x = (float)pos.x / (float)mTexWidth;
		sprite->trimStart.y = (float)pos.y / (float)mTexHeight;
	}

	void Sprite2DComponent::SetTrimmingEndPos(const Vector2i &pos)
	{
		/*
			mTextureVK->sprite->trimEnd.x = (float)pos.x / (float)mTexWidth;
			mTextureVK->sprite->trimEnd.y = (float)pos.y / (float)mTexHeight;
			if (mTextureVK->sprite->isChangeBuffer == false)
			{
				mTextureVK->buffer = mOwner.GetScene()->GetRenderer()->GetVK().CreateBuffer(
					sizeof(Vertex), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
				mTextureVK->sprite->isChangeBuffer = true;
			}
			*/
		sprite->isChangeBuffer = true;
		mOwner.GetScene().GetRenderer()->ChangeBufferSprite(sprite);
		sprite->trimEnd.x = (float)pos.x / (float)mTexWidth;
		sprite->trimEnd.y = (float)pos.y / (float)mTexHeight;
	}
}