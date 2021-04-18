#pragma once
#include "Actor.h"

namespace nen
{
	class Animation2DSprite : public Actor
	{
	public:
		Animation2DSprite(std::shared_ptr<class Scene> scene, const int height, const int width, const float framePerSec, const std::string &path);
		void UpdateActor(float deltaTime) override;
		void Init();
		void SetReverse(bool isReverse) { misReverse = isReverse; }

	private:
		void Calc();
		bool misReverse = false;
		Vector2i mTexSize;
		Vector2i firstPos;
		int mRow = 0;
		int mColumn = 0;
		const int mPicCounter;
		int mNowPos = 0;
		int mNowColumn = 1;
		float mframePerSec = 0.f;
		float bufTime = 0.f;
		std::shared_ptr<class Sprite2DComponent> mImage = nullptr;
	};
}