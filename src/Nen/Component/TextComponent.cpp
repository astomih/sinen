#include <Nen.hpp>
#include <iostream>
namespace nen
{
	TextComponent::TextComponent(Actor &actor, int drawOrder) : Draw2DComponent(actor, drawOrder)
	{
	}

	void TextComponent::SetString(const std::string &str, const Color &color)
	{
		if (!mFont->isLoaded())
		{
			mFont->LoadFromFile("Assets/Font/mplus/mplus-1p-medium.ttf", 32);
		}

		if (this->GetSprite())
		{
			GetActor().GetScene().GetRenderer()->RemoveDrawObject2D(this->GetSprite());
			this->GetSprite().reset();
		}
		Create(mFont->RenderText(str, color));
	}
	void TextComponent::SetFont(std::shared_ptr<Font> font)
	{
		mFont = font;
	}
}