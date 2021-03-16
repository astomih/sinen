#include <Components.hpp>
#include <Actors.hpp>
#include <Engine.hpp>
#include <Scene.hpp>
#include <iostream>
#include <SDL_image.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

constexpr int matrixSize = sizeof(float) * 16;

Matrix4 CreateWorld(const Matrix4 &scale, const Quaternion &rotate, const Vector3f &position)
{
	return Matrix4::CreateTranslation(position) * scale * Matrix4::CreateScale(1.f) * Matrix4::CreateFromQuaternion(rotate);
};
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
			SetTexture(tex);
		}
		float scaleOwner = mOwner.GetScale();
		auto translate = glm::translate(glm::identity<glm::mat4>(), glm::vec3(mOwner.GetPosition().x, mOwner.GetPosition().y, 0));
		glm::quat quat;
		quat.x = mOwner.GetRotation().x;
		quat.y = mOwner.GetRotation().y;
		quat.z = mOwner.GetRotation().z;
		quat.w = mOwner.GetRotation().w;
		auto rotate = glm::mat4_cast(quat);
		auto scale = glm::scale(glm::identity<glm::mat4>(), glm::vec3(mTexture->GetWidth() * scaleOwner, mTexture->GetHeight() * scaleOwner, 1));

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

void Sprite2DComponent::SetTexture(std::shared_ptr<Texture> texture, const float scale)
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
	if (renderer->GetGraphicsAPI() == GraphicsAPI::Vulkan)
	{
		mTextureVK = std::make_shared<SpriteVK>();
		renderer->GetVK().registerImageObject(mTexture);
		mTextureVK->mTexture = mTexture;
		mTextureVK->drawOrder = mDrawOrder;
		renderer->GetVK().registerTexture(mTextureVK, mTexture->id, TextureType::Image2D);

		auto world = CreateWorld(Matrix4::CreateScale(mTexture->GetWidth(), mTexture->GetHeight(), 1), Quaternion(Vector3f::UnitZ, 0), mOwner.GetPosition());
		glm::mat4x4 viewproj(0);
		viewproj[0][0] = 1.f / Window::Size.x;
		viewproj[1][1] = 1.f / Window::Size.y;
		viewproj[2][2] = 1.f;
		viewproj[3][3] = 1.f;

		mTextureVK->param.proj = viewproj;
		mTextureVK->param.view = glm::identity<glm::mat4>();
		memcpy(glm::value_ptr(mTextureVK->param.world), world.GetAsFloatPtr(), matrixSize);
	}
	if (mOwner.GetScene()->GetRenderer()->GetGraphicsAPI() == GraphicsAPI::OpenGL)
	{

		mOwner.GetScene()->GetRenderer()->GetGL().registerTexture(mTexture, TextureType::Image2D);
		sprite = std::make_shared<SpriteGL>();
		sprite->drawOrder = mDrawOrder;
		sprite->textureIndex = mTexture->id;
		mTexWidth = mTexture->GetWidth();
		mTexHeight = mTexture->GetHeight();
		auto world = CreateWorld(
			Matrix4::CreateScale(
				mTexture->GetWidth(),
				mTexture->GetHeight(), 1),
			Quaternion(Vector3f::UnitZ, 0),
			mOwner.GetPosition());
		glm::mat4x4 viewproj(0);
		viewproj[0][0] = 1.f / Window::Size.x;
		viewproj[1][1] = 1.f / Window::Size.y;
		viewproj[2][2] = 1.f;
		viewproj[3][3] = 1.f;

		sprite->param.proj = viewproj;
		sprite->param.view = glm::identity<glm::mat4>();
		memcpy(glm::value_ptr(sprite->param.world), world.GetAsFloatPtr(), matrixSize);
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
