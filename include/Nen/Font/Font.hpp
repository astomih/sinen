#ifndef NEN_FONT_HPP
#define NEN_FONT_HPP
#include "../Math/Vector3.hpp"

namespace
{
	struct _TTF_Font;
}

namespace nen
{
	/*
		フォントクラス
	*/
	class Font
	{
	public:
		Font() :font(nullptr), isLoad(false), pointSize(0) {};
		~Font() = default;

		/*
			フォントの描画クオリティ
		*/
		enum class Quality
		{
			// 荒いが高速。背景あり
			Solid,
			// 高品質・高速だが背景あり
			Shaded,
			// 高品質で背景が無いが遅い
			Blended
		};

		// ファイルからフォントを読み込む
		bool LoadFromFile(std::string_view fontName, int pointSize);
		bool isLoaded() { return isLoad; }
		// フォントを解放
		void Unload();

		// Textureにフォントを書き込む
		std::shared_ptr<Texture> RenderText(std::string_view text, const Color& color = Palette::White, Quality quality = Quality::Blended,
			const Color& backgroundColor = Palette::Black);

	private:

		int pointSize;
		bool isLoad;
		std::string fontName;
		::_TTF_Font* font;
	};
}
#endif