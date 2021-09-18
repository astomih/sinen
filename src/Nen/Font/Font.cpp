#include <SDL_ttf.h>
#include <SDL.h>
#include <Nen.hpp>
#include <cassert>
#include "../Texture/SurfaceHandle.hpp"
namespace nen
{
	bool Font::LoadFromFile(std::string_view fontName, int pointSize)
	{
		this->fontName = fontName;
		this->pointSize = pointSize;
		font = ::TTF_OpenFontRW((SDL_RWops *)AssetReader::LoadAsRWops(AssetType::Font, this->fontName), 1, this->pointSize);
		if (!font)
		{
			Logger::Error("%s", TTF_GetError());
			return false;
		}
		return (isLoad = true);
	}

	std::shared_ptr<Texture> Font::RenderText(std::string_view text, const Color &color, Quality quality, const Color &backgroundColor)
	{
		auto texture = std::make_shared<Texture>();
		//My Color to SDL_Color
		SDL_Color sdlColor;
		sdlColor.r = static_cast<Uint8>(color.r * 255);
		sdlColor.g = static_cast<Uint8>(color.g * 255);
		sdlColor.b = static_cast<Uint8>(color.b * 255);
		sdlColor.a = 255;
		texture->id = std::string(text);
		::SDL_Surface *surf = nullptr;
		switch (quality)
		{
		case nen::Font::Quality::Solid:
			surf = ::TTF_RenderUTF8_Solid(font, std::string(text).c_str(), sdlColor);
			break;
		case nen::Font::Quality::Shaded:
		{
			SDL_Color bg;
			bg.r = static_cast<Uint8>(color.r * 255);
			bg.g = static_cast<Uint8>(color.g * 255);
			bg.b = static_cast<Uint8>(color.b * 255);
			bg.a = 255;
			surf = ::TTF_RenderUTF8_Shaded(font, std::string(text).c_str(), sdlColor, bg);
		}
		break;
		case nen::Font::Quality::Blended:
			surf = ::TTF_RenderUTF8_Blended(font, std::string(text).c_str(), sdlColor);
			break;
		default:
			break;
		}
		texture->SetWidth(surf->w);
		texture->SetHeight(surf->h);
		SurfaceHandle::Register(text, std::move(std::unique_ptr<::SDL_Surface, SDLObjectCloser>(surf)));
		return texture;
	}
}
