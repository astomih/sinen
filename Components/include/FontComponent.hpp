#pragma once
#include <string>
#include <memory>
namespace nen
{
	class FontComponent : public Sprite2DComponent
	{
	public:
		FontComponent(class Actor&, int drawOrder = 10000);
		void SetString(const std::string&, const class Vector3f& color = Color::White, int drawSize = 30);
		void SetFont(std::shared_ptr<class Font> font);

	private:
		std::shared_ptr<class Font> mFont;
	};
}