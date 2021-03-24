#pragma once
#include "Actor.h"
#include <string>
namespace nen
{
	class Text : public Actor
	{
	public:
		Text(std::shared_ptr<class Scene> scene);
		void SetText(const std::string& text, const int size = 10) const;
		int GetWidth() const;
		int GetHeight() const;
		void SetColor(const Color::Color& color);

	private:
		std::unique_ptr<class FontComponent> mFont;
	};
}