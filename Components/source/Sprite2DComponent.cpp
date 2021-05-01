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
		mOwner.GetScene()->GetRenderer()->AddSprite2D(this);
	}

	void Sprite2DComponent::Update(float deltaTime)
	{
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

			if (mOwner.GetScene()->GetRenderer()->GetGraphicsAPI() == GraphicsAPI::Vulkan)
			{
				mTextureVK->param.world = world;
			}
			if (mOwner.GetScene()->GetRenderer()->GetGraphicsAPI() == GraphicsAPI::OpenGL)
			{
				sprite->param.world = world;
			}
		}
	}

	Sprite2DComponent::~Sprite2DComponent()
	{
		mOwner.GetScene()->GetRenderer()->RemoveSprite2D(this);
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
		auto renderer = mOwner.GetScene()->GetRenderer();
		auto api = renderer->GetGraphicsAPI();
		auto scaleOwner = mOwner.GetScale();
		Matrix4 viewproj = Matrix4::Identity;
		viewproj.mat[0][0] = 1.f / Window::Size.x;
		viewproj.mat[1][1] = 1.f / Window::Size.y;

		if (renderer->GetGraphicsAPI() == GraphicsAPI::Vulkan)
		{
			mTextureVK = std::make_shared<vk::SpriteVK>();
			renderer->GetVK().registerImageObject(mTexture);
			mTextureVK->mTexture = mTexture;
			mTextureVK->drawOrder = mDrawOrder;
			mTextureVK->vertexIndex = shape.data();
			renderer->GetVK().registerTexture(mTextureVK, mTexture->id, TextureType::Image2D);

			mTextureVK->param.world = mOwner.GetWorldTransform();
			mTextureVK->param.proj = viewproj;
			mTextureVK->param.view = Matrix4::Identity;
		}
		if (mOwner.GetScene()->GetRenderer()->GetGraphicsAPI() == GraphicsAPI::OpenGL)
		{

			mOwner.GetScene()->GetRenderer()->GetGL().registerTexture(mTexture, TextureType::Image2D);
			sprite = std::make_shared<Sprite>();
			sprite->drawOrder = mDrawOrder;
			sprite->textureIndex = mTexture->id;
			sprite->vertexIndex = shape.data();

			sprite->param.world = mOwner.GetWorldTransform();
			sprite->param.proj = viewproj;
			sprite->param.view = Matrix4::Identity;

			mOwner.GetScene()->GetRenderer()->GetGL().pushSprite2d(sprite);
		}
	}
	void Sprite2DComponent::SetTrimmingStartPos(const Vector2i &pos)
	{
		if (mOwner.GetScene()->GetRenderer()->GetGraphicsAPI() == GraphicsAPI::Vulkan)
		{
			mTextureVK->trimStart.x = (float)pos.x / (float)mTexWidth;
			mTextureVK->trimStart.y = (float)pos.y / (float)mTexHeight;
			if (mTextureVK->isChangeBuffer == false)
			{
				mTextureVK->buffer = mOwner.GetScene()->GetRenderer()->GetVK().CreateBuffer(
					sizeof(Vertex) * 4, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
				mTextureVK->isChangeBuffer = true;
			}
		}
		else
		{
			sprite->isChangeBuffer = true;
			sprite->trimStart.x = (float)pos.x / (float)mTexWidth;
			sprite->trimStart.y = (float)pos.y / (float)mTexHeight;
		}
	}

	void Sprite2DComponent::SetTrimmingEndPos(const Vector2i &pos)
	{
		if (mOwner.GetScene()->GetRenderer()->GetGraphicsAPI() == GraphicsAPI::Vulkan)
		{
			mTextureVK->trimEnd.x = (float)pos.x / (float)mTexWidth;
			mTextureVK->trimEnd.y = (float)pos.y / (float)mTexHeight;
			if (mTextureVK->isChangeBuffer == false)
			{
				mTextureVK->buffer = mOwner.GetScene()->GetRenderer()->GetVK().CreateBuffer(
					sizeof(Vertex), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
				mTextureVK->isChangeBuffer = true;
			}
		}
		else
		{
			sprite->isChangeBuffer = true;
			sprite->trimEnd.x = (float)pos.x / (float)mTexWidth;
			sprite->trimEnd.y = (float)pos.y / (float)mTexHeight;
		}
	}
}