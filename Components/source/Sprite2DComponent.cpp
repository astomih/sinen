#include <Components.hpp>
#include <Actors.hpp>
#include <Engine.hpp>
#include <Scene.hpp>
#include <iostream>
#include <SDL_image.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
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
		if (mOwner.isRecompute())
		{
			if (!mTexture)
			{

				auto tex = std::make_shared<Texture>();
				tex->Load("Assets/Default.png");
				Create(tex);
			}
			float scaleOwner = mOwner.GetScale();
			auto translate = glm::translate(glm::identity<glm::mat4>(), glm::vec3(mOwner.GetPosition().x * 2.f, mOwner.GetPosition().y * 2.f, 0.f));

			glm::quat quat;
			auto rotate = glm::mat4(0);
			Matrix4 mat = Matrix4::CreateFromQuaternion(mOwner.GetRotation());
			memcpy(&rotate, &mat, sizeof(float) * 16);
			glm::mat4x4 scale = glm::scale(glm::identity<glm::mat4x4>(), glm::vec3(scaleOwner * mTexture->GetWidth(), mTexture->GetHeight() * scaleOwner, 0.f));

			if (mOwner.GetScene()->GetRenderer()->GetGraphicsAPI() == GraphicsAPI::Vulkan)
			{
				mTextureVK->param.world = translate * rotate * scale;
			}
			if (mOwner.GetScene()->GetRenderer()->GetGraphicsAPI() == GraphicsAPI::OpenGL)
			{
				sprite->param.world = translate * rotate * scale;
			}
			mOwner.RecomuteFinished();
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
		float scaleOwner = mOwner.GetScale();
		//auto translate = glm::translate(glm::identity<glm::mat4>(), glm::vec3(mOwner.GetPosition().x, mOwner.GetPosition().y, 0));
		auto translate = glm::identity<glm::mat4>();
		translate[3][0] = mOwner.GetPosition().x;
		translate[3][1] = mOwner.GetPosition().y;

		glm::quat quat;
		memcpy(&quat, &mOwner.GetRotation(), sizeof(Quaternion));
		auto rotate = glm::mat4_cast(quat);
		glm::mat4x4 scale2(0);
		scale2[0][0] = scaleOwner;
		scale2[1][1] = scaleOwner;
		scale2[2][2] = 1.f;
		scale2[3][3] = 1.f;
		static glm::mat4x4 viewproj = glm::ortho(-Window::Size.x, Window::Size.x, -Window::Size.y, Window::Size.y);
		if (renderer->GetGraphicsAPI() == GraphicsAPI::Vulkan)
		{
			mTextureVK = std::make_shared<vk::SpriteVK>();
			renderer->GetVK().registerImageObject(mTexture);
			mTextureVK->mTexture = mTexture;
			mTextureVK->drawOrder = mDrawOrder;
			mTextureVK->vertexIndex = shape.data();
			renderer->GetVK().registerTexture(mTextureVK, mTexture->id, TextureType::Image2D);

			mTextureVK->param.world = translate * rotate * scale2;
			mTextureVK->param.proj = viewproj;
			mTextureVK->param.view = glm::identity<glm::mat4>();
		}
		if (mOwner.GetScene()->GetRenderer()->GetGraphicsAPI() == GraphicsAPI::OpenGL)
		{

			mOwner.GetScene()->GetRenderer()->GetGL().registerTexture(mTexture, TextureType::Image2D);
			sprite = std::make_shared<Sprite>();
			sprite->drawOrder = mDrawOrder;
			sprite->textureIndex = mTexture->id;
			sprite->vertexIndex = shape.data();

			sprite->param.world = translate * rotate * scale2;
			sprite->param.proj = viewproj;
			sprite->param.view = glm::identity<glm::mat4>();
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