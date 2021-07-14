#include <Engine.hpp>
namespace nen
{
	bool Font::Load(const std::string& fileName)
	{
		//Support font sizes
		std::vector<int> fontSizes =
		{
			8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28,
			30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 52, 56,
			60, 64, 68, 72 };

		//Call TTF_OpenFont func per fontSizes
		for (auto& size : fontSizes)
		{
			::TTF_Font* font = ::TTF_OpenFont(fileName.c_str(), size);
			if (!font)
			{
				::SDL_Log("font %s couldn't be loaded.", fileName.c_str());
				return false;
			}
			mFontData.emplace(size, font);
		}
		loaded = true;
		return true;
	}

	std::shared_ptr<Texture> Font::RenderText(const std::string& text, const Vector3f& color, int pointSize)
	{
		auto texture = std::make_shared<Texture>();
		//My Color to SDL_Color
		SDL_Color sdlColor;
		sdlColor.r = static_cast<Uint8>(color.x * 255);
		sdlColor.g = static_cast<Uint8>(color.y * 255);
		sdlColor.b = static_cast<Uint8>(color.z * 255);
		sdlColor.a = 255;
		//Find pointSize Data
		auto itr = mFontData.find(pointSize);
		if (itr != mFontData.end())
		{
			::TTF_Font* font = itr->second;
			//Render to SDL_Surface（Alpha blending)
			auto surf = std::unique_ptr<::SDL_Surface, SDLObjectCloser>(::TTF_RenderUTF8_Blended(font, text.c_str(), sdlColor));
			if (surf)
			{
				texture->SetSurface(std::move(surf));
			}
		}
		else
		{
			assert(TTF_GetError());
		}
		texture->id = text;

		return texture;
	}

	std::shared_ptr<Texture> Font::RenderText(const std::string& text, SupportFontSizes pointSize, const Vector3f& color)
	{
		auto texture = std::make_shared<Texture>();
		//My Color to SDL_Color
		SDL_Color sdlColor;
		sdlColor.r = static_cast<Uint8>(color.x * 255);
		sdlColor.g = static_cast<Uint8>(color.y * 255);
		sdlColor.b = static_cast<Uint8>(color.z * 255);
		sdlColor.a = 255;
		//Find pointSize Data
		int size;
		switch (pointSize)
		{
		case SupportFontSizes::S8:
			size = 8;
			break;
		case SupportFontSizes::S9:
			size = 9;
			break;
		case SupportFontSizes::S10:
			size = 10;
			break;
		case SupportFontSizes::S11:
			size = 11;
			break;
		case SupportFontSizes::S12:
			size = 12;
			break;
		case SupportFontSizes::S14:
			size = 14;
			break;
		case SupportFontSizes::S16:
			size = 16;
			break;
		case SupportFontSizes::S18:
			size = 18;
			break;
		case SupportFontSizes::S20:
			size = 20;
			break;
		case SupportFontSizes::S22:
			size = 22;
			break;
		case SupportFontSizes::S24:
			size = 24;
			break;
		case SupportFontSizes::S26:
			size = 26;
			break;
		case SupportFontSizes::S28:
			size = 28;
			break;
		case SupportFontSizes::S30:
			size = 30;
			break;
		case SupportFontSizes::S32:
			size = 32;
			break;
		case SupportFontSizes::S34:
			size = 34;
			break;
		case SupportFontSizes::S36:
			size = 36;
			break;
		case SupportFontSizes::S38:
			size = 38;
			break;
		case SupportFontSizes::S40:
			size = 40;
			break;
		case SupportFontSizes::S42:
			size = 42;
			break;
		case SupportFontSizes::S44:
			size = 44;
			break;
		case SupportFontSizes::S46:
			size = 46;
			break;
		case SupportFontSizes::S48:
			size = 48;
			break;
		case SupportFontSizes::S52:
			size = 52;
			break;
		case SupportFontSizes::S56:
			size = 56;
			break;
		case SupportFontSizes::S60:
			size = 60;
			break;
		case SupportFontSizes::S64:
			size = 64;
			break;
		case SupportFontSizes::S68:
			size = 68;
			break;
		case SupportFontSizes::S72:
			size = 72;
			break;
		default:
			size = 0;
			throw 0;
			break;
		}
		auto itr = mFontData.find(size);
		if (itr != mFontData.end())
		{
			::TTF_Font* font = itr->second;
			//Render to SDL_Surface（Alpha blending)
			auto surf = std::unique_ptr<::SDL_Surface, SDLObjectCloser>(::TTF_RenderUTF8_Blended(font, text.c_str(), sdlColor));

			if (surf)
			{
				texture->SetSurface(std::move(surf));
			}
		}
		else
		{
		}
		return texture;
		return nullptr;
	}
}
