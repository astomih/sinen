#include <Components.hpp>
#include <Actors.hpp>
#include <Engine.hpp>
#include <iostream>
#include <Scene.hpp>
namespace nen
{
	FontComponent::FontComponent(Actor &actor, int drawOrder) : Sprite2DComponent(actor, drawOrder)
	{
	}

	void FontComponent::SetString(const std::string &str, const Vector3f &color, int pointSize)
	{
		try
		{
			if (!mFont)
				throw std::runtime_error("Error: Font set yet.");
		}
		catch (std::runtime_error &e)
		{
			std::cout << e.what() << std::endl;
		}
		if (!mFont->isLoaded())
		{
			mFont->Load("Assets/Font/mplus/mplus-1p-medium.ttf");
		}

		if (this->GetSprite())
		{
			GetActor().GetScene().GetRenderer()->RemoveSprite2D(this->GetSprite());
			this->GetSprite().reset();
		}
		Create(mFont->RenderText(str, color, pointSize));
	}
	void FontComponent::SetFont(std::shared_ptr<Font> font)
	{
		mFont = font;
	}
}