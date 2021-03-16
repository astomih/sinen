#include <Components.hpp>
#include <Actors.hpp>
#include <Engine.hpp>
#include <iostream>
std::shared_ptr<Font> FontComponent::mFont = std::shared_ptr<Font>(std::make_shared<Font>());
FontComponent::FontComponent(Actor& actor, int drawOrder) :Sprite2DComponent(actor, drawOrder)
{
	if (!mFont->isLoaded())
	{
		mFont->Load("Assets/mplus-1p-medium.ttf");
	}
}

void FontComponent::SetString(const std::string& str, const Vector3f& color, int pointSize)
{
	SetTexture(mFont->RenderText(str, color, pointSize));
}