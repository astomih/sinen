#include <Components.hpp>
#include <Actors.hpp>
#include <Engine.hpp>
#include <iostream>
#include <Scene.hpp>
namespace nen
{
	std::shared_ptr<Font> FontComponent::mFont = std::shared_ptr<Font>(std::make_shared<Font>());
	FontComponent::FontComponent(Actor &actor, int drawOrder) : Sprite2DComponent(actor, drawOrder)
	{
		if (!mFont->isLoaded())
		{
			mFont->Load("Assets/mplus-1p-medium.ttf");
		}
	}

	void FontComponent::SetString(const std::string &str, const Vector3f &color, int pointSize)
	{

		if (this->GetSprite())
		{
			GetActor().GetScene().GetRenderer()->RemoveSprite2D(this->GetSprite());
			this->GetSprite().reset();
		}
		Create(mFont->RenderText(str, color, pointSize));
	}
}