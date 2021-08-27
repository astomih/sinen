#pragma once
#include "../Math/Vector2.hpp"
#include "../Texture/Texture.hpp"
#include "../Color/Color.hpp"
#include "Component.hpp"
#include <utility>

namespace nen
{
	class Draw2DComponent : public Component
	{
	public:
		// (Lower draw order corresponds with further back)
		Draw2DComponent(class Actor &owner, int drawOrder = 100, Texture tex = Texture());
		~Draw2DComponent();

		virtual void Create(std::shared_ptr<Texture> texture, float scale = 1.0f, std::string_view shape = "SPRITE");
		virtual void Update(float deltaTime) override;

		int GetDrawOrder() const { return mDrawOrder; }
		int GetTexHeight() const { return mTexHeight; }
		int GetTexWidth() const { return mTexWidth; }

		std::shared_ptr<class DrawObject> GetSprite() { return this->sprite; }

		void SetScale(const Vector2 &scale)
		{
			mTexWidth = static_cast<int>(mTexWidth * scale.x);
			mTexHeight = static_cast<int>(mTexHeight * scale.y);
		}

	private:
		std::shared_ptr<class DrawObject> sprite;
		int mDrawOrder = 0;
		int mTexWidth = 0;
		int mTexHeight = 0;
		std::shared_ptr<class Texture> mTexture;
	};
}
