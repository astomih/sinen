#pragma once
#include <Math/Vector3.hpp>
#include <Color/Color.hpp>
#include "Sprite2DComponent.hpp"
#include <string>
#include <memory>
namespace nen
{
	class FontComponent : public Sprite2DComponent
	{
	public:
		FontComponent(class Actor &, int drawOrder = 10000);
		void SetString(const std::string &, const class Color &color = Palette::White, int drawSize = 30);
		void SetFont(std::shared_ptr<class Font> font);

	private:
		std::shared_ptr<class Font> mFont;
	};
}