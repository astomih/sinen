#include <nen.hpp>
namespace nen
{
	Text::Text(Scene& scene)
		: Actor(scene)
	{
		mFont = std::make_shared<FontComponent>(*this);
		this->AddComponent(mFont);
	}

	void Text::SetText(const std::string &text, const int size) const
	{
		mFont->SetString(text, Color::White, size);
	}

	int Text::GetWidth() const
	{
		return mFont->GetTexWidth();
	}

	int Text::GetHeight() const
	{
		return mFont->GetTexHeight();
	}

	void Text::SetColor(const Color::Color &color)
	{
		mFont->SetColor(color);
	}
}