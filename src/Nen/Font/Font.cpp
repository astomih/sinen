#include <SDL_ttf.h>
#include <Nen.hpp>
#include <cassert>
namespace nen
{
	bool Font::Load(std::string_view fontName, int pointSize)
	{
		this->fontName = fontName;
		this->pointSize = pointSize;
		font = ::TTF_OpenFont(std::string(this->fontName).c_str(), this->pointSize);
		if (!font)
		{
			Logger::Error("%s", TTF_GetError());
			return false;
		}
		return (isLoad = true);
	}


	std::shared_ptr<Texture> Font::RenderText(std::string_view text, const Color& color, Quality quality, const Color& backgroundColor)
	{
		auto texture = std::make_shared<Texture>();
		//My Color to SDL_Color
		SDL_Color sdlColor;
		sdlColor.r = static_cast<Uint8>(color.r * 255);
		sdlColor.g = static_cast<Uint8>(color.g * 255);
		sdlColor.b = static_cast<Uint8>(color.b * 255);
		sdlColor.a = 255;
		texture->id = text;
		std::unique_ptr<::SDL_Surface, SDLObjectCloser> surf;
		switch (quality)
		{
		case nen::Font::Quality::Solid:
			surf = std::unique_ptr<::SDL_Surface, SDLObjectCloser>(::TTF_RenderUTF8_Solid(font, std::string(text).c_str(), sdlColor));
			break;
		case nen::Font::Quality::Shaded:
		{
			SDL_Color bg;
			bg.r = static_cast<Uint8>(color.r * 255);
			bg.g = static_cast<Uint8>(color.g * 255);
			bg.b = static_cast<Uint8>(color.b * 255);
			bg.a = 255;
			surf = std::unique_ptr<::SDL_Surface, SDLObjectCloser>(::TTF_RenderUTF8_Shaded(font, std::string(text).c_str(), sdlColor, bg));
		}
		break;
		case nen::Font::Quality::Blended:
			surf = std::unique_ptr<::SDL_Surface, SDLObjectCloser>(::TTF_RenderUTF8_Blended(font, std::string(text).c_str(), sdlColor));
			break;
		default:
			break;
		}
		if (surf)
		{
			texture->SetSurface(std::move(surf));
		}
		else
		{
			Logger::Error("Font RenderError:%s", TTF_GetError());
		}
		return texture;
	}
}
