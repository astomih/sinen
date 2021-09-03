#pragma once
#include "../Math/Vector3.hpp"
#include "../Color/Color.hpp"
#include "Draw2DComponent.hpp"
#include <string>
#include <memory>
namespace nen
{
	class TextComponent : public Draw2DComponent
	{
	public:
		TextComponent(class Actor &, int drawOrder = 10000);
		void SetString(const std::string &, const class Color &color = Palette::White);
		void SetFont(std::shared_ptr<class Font> font);

	private:
		std::shared_ptr<class Font> mFont;
	};
}