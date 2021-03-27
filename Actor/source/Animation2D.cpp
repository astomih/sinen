#include <Actors.hpp>
#include <Scene.hpp>
#include <Components.hpp>
#include <Engine.hpp>
namespace nen
{
	Animation2DSprite::Animation2DSprite(std::shared_ptr<Scene> scene, const int height, const int width, const float framePerSec, const std::string& path)
		: Actor(scene),
		mRow(height),
		mColumn(width),
		mframePerSec(framePerSec),
		mPicCounter(height* width),
		mImage(std::make_shared<Sprite2DComponent>(*this, 5000))
	{
		this->AddComponent(mImage);
		auto tex = std::make_shared<Texture>();
		if (tex->Load(path))
		{
			mImage->Create(tex);
			mTexSize.x = tex->GetWidth();
			mTexSize.y = tex->GetHeight();
			mImage->SetTrimmingStartPos(Vector2i(mTexSize.x / 2.f, mTexSize.y));
			mImage->SetTrimmingEndPos(Vector2i(mTexSize.x, 0));
		}
		else
		{
			assert("Animation failed.");
		}
	}

	Animation2DSprite::~Animation2DSprite()
	{
		this->GetScene()->RemoveActor(shared_from_this());
	}

	void Animation2DSprite::UpdateActor(float deltaTime)
	{
	}

	void Animation2DSprite::Init()
	{
	}

	void Animation2DSprite::Calc()
	{
		const Vector2i topleft(firstPos.x - mTexSize.x / 2, firstPos.y + mTexSize.y / 2);
		const Vector2i calctrim(topleft.x + (mNowPos % mColumn) * (mTexSize.x / mColumn), topleft.y - (mNowColumn) * (mTexSize.y / (mRow)));

		if (misReverse)
		{
			Vector2i center(calctrim.x + (mTexSize.x / mColumn / 2), calctrim.y + (mTexSize.y / mRow / 2));
			Vector2i diffrence = center - firstPos;
			mImage->SetTrimmingStartPos(Vector2i(calctrim.x - diffrence.x, calctrim.y - diffrence.y));
		}
		else
		{
			const Vector2i length1per4((mTexSize.x / mColumn / 2), (mTexSize.y / mRow / 2));
			const Vector2i center(calctrim + length1per4);
			const Vector2i difference(firstPos - center);
			const Vector2i position(difference.x + firstPos.x, difference.y + firstPos.y);
		}
	}
}