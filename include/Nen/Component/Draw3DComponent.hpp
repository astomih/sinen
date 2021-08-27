#pragma once
#include "../Math/Vector2.hpp"
#include "Component.hpp"
namespace nen
{
	class Draw3DComponent : public Component
	{
	public:
		Draw3DComponent(class Actor &owner, int drawOrder = 100);
		~Draw3DComponent();

		void Update(float deltaTime) override;

		int GetTexHeight() const { return mTexHeight; }
		int GetTexWidth() const { return mTexWidth; }
		std::shared_ptr<class DrawObject> GetSprite() { return this->sprite; }
		void Create(std::shared_ptr<class Texture> texture, std::string_view shape = "SPRITE");
		std::shared_ptr<class DrawObject> sprite;

	private:
		int mTexWidth = 0;
		int mTexHeight = 0;
		Actor &mOwner;
		std::shared_ptr<class Texture> mTexture;
	};
}
