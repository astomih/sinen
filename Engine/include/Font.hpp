#pragma once
#include "Math.hpp"
#include <SDL_ttf.h>
#include <unordered_map>
#include "Texture.h"

namespace nen
{
	enum class SupportFontSizes : int
	{
		S8,
		S9,
		S10,
		S11,
		S12,
		S14,
		S16,
		S18,
		S20,
		S22,
		S24,
		S26,
		S28,
		S30,
		S32,
		S34,
		S36,
		S38,
		S40,
		S42,
		S44,
		S46,
		S48,
		S52,
		S56,
		S60,
		S64,
		S68,
		S72
	};

	class Font
	{
	public:
		Font();
		~Font()
		{
			Unload();
		}
		//File Load/Unload
		bool Load(const std::string& fileName);
		bool isLoaded() { return loaded; }
		void Unload()
		{
			for (const auto& i : mFontData)
			{
				if (i.second)
					TTF_CloseFont(i.second);
			}
		}

		//Render string
		class std::shared_ptr<Texture> RenderText(const std::string& text, const Vector3f& color = Color::White, int pointSize = 30);
		class std::shared_ptr<Texture> RenderText(const std::string& text, SupportFontSizes pointSize, const Vector3f& color = Color::White);

	private:
		std::unordered_map<int, TTF_Font*> mFontData;
		bool loaded;
		using S1 = char;
		const S1 padding[3];
	};
}