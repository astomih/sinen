#ifndef NEN_FONT_HPP
#define NEN_FONT_HPP
#include "../Math/Vector3.hpp"
#include "../Texture/Texture.hpp"
#include <memory>

namespace {
struct _TTF_Font;
}

namespace nen {
/*
        フォントクラス
*/
class font {
public:
  font() : m_font(nullptr), isLoad(false), pointSize(0){};
  ~font() = default;

  /*
          フォントの描画クオリティ
  */
  enum class quality {
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
  std::shared_ptr<texture>
  RenderText(std::string_view text, const color &_color = palette::White,
             quality _quality = quality::Blended,
             const color &backgroundColor = palette::Black);

private:
  int pointSize;
  bool isLoad;
  std::string fontName;
  ::_TTF_Font *m_font;
};
} // namespace nen
#endif