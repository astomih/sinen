#pragma once
#include "../Math/Vector2.hpp"
#include "../Texture/Texture.hpp"
#include "../Color/Color.hpp"
#include <utility>

namespace nen
{
	class Sprite2DComponent : public Component
	{
	public:
		// (Lower draw order corresponds with further back)
		Sprite2DComponent(class Actor &owner, int drawOrder = 100, Texture tex = Texture());
		~Sprite2DComponent();

		virtual void Create(std::shared_ptr<Texture> texture, float scale = 1.0f, std::string_view shape = "SPRITE");
		virtual void Update(float deltaTime) override;

		int GetDrawOrder() const { return mDrawOrder; }
		int GetTexHeight() const { return mTexHeight; }
		int GetTexWidth() const { return mTexWidth; }

		std::shared_ptr<class Sprite> GetSprite() { return this->sprite; }

		void SetReverse(bool Use) { mUseReverse = Use; }
		void SetFlip(bool Use) { mUseFlip = Use; }
		void SetScale(const Vector2 &scale)
		{
			mTexWidth = static_cast<int>(mTexWidth * scale.x);
			mTexHeight = static_cast<int>(mTexHeight * scale.y);
		}
		void SetUseTrimming(const bool isuse) { mUseTrim = isuse; }
		void SetTrimmingStartPos(int x, int y);
		void SetTrimmingEndPos(int x, int y);
		void SetBoolean(bool boolean) { mBoolean = boolean; }
		void SetColor(const Color &color) { mColor = color; }
		Color GetColor() { return mColor; }

	private:
		std::shared_ptr<class Sprite> sprite;
		int mDrawOrder = 0;
		int mTexWidth = 0;
		int mTexHeight = 0;
		bool mUseReverse = false;
		bool mUseFlip = false;
		bool mBoolean = false;
		Vector2 vuv = Vector2(1.f, 1.f);
		Color mColor = Color(1.f, 1.f, 1.f, 1.f);
		bool mUseTrim = false;
		std::pair<int, int> trim_s;
		std::pair<int, int> trim_e;
		std::shared_ptr<class Texture> mTexture;
	};
}
