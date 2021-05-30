#pragma once
#include <SDL.h>
#include <Engine.hpp>

namespace nen
{
	class Sprite2DComponent : public Component
	{
	public:
		// (Lower draw order corresponds with further back)
		Sprite2DComponent(class Actor& owner, int drawOrder = 100, Texture tex = Texture());
		~Sprite2DComponent();

		virtual void Draw(class Shader* shader);
		virtual void Create(std::shared_ptr<Texture> texture, float scale = 1.0f, std::string_view shape = "SPRITE");
		virtual void Update(float deltaTime) override;

		int GetDrawOrder() const { return mDrawOrder; }
		int GetTexHeight() const { return mTexHeight; }
		int GetTexWidth() const { return mTexWidth; }

		void SetReverse(bool Use) { mUseReverse = Use; }
		void SetFlip(bool Use) { mUseFlip = Use; }
		void SetScale(const Vector2f& scale)
		{
			mTexWidth = static_cast<int>(mTexWidth * scale.x);
			mTexHeight = static_cast<int>(mTexHeight * scale.y);
		}
		void SetUseTrimming(const bool isuse) { mUseTrim = isuse; }
		void SetTrimmingStartPos(const Vector2i& pos);
		void SetTrimmingEndPos(const Vector2i& pos);
		void SetBoolean(bool boolean) { mBoolean = boolean; }
		void SetColor(const Color::Color& color) { mColor = color; }
		Color::Color GetColor() { return mColor; }

	private:
		std::shared_ptr<class Sprite> sprite;
		int mDrawOrder = 0;
		int mTexWidth = 0;
		int mTexHeight = 0;
		bool mUseReverse = false;
		bool mUseFlip = false;
		bool mBoolean = false;
		Vector2f vuv = Vector2f(1.f, 1.f);
		Color::Color mColor = Color::Color(1.f, 1.f, 1.f, 1.f);
		bool mUseTrim = false;
		Vector2i trim_s;
		Vector2i trim_e;
		std::shared_ptr<Texture> mTexture;
	};
}
