#pragma once
#include "../Color/Color.hpp"
#include "../Math/Vector2.hpp"
#include "../Texture/Texture.hpp"
#include "../Utility/handler.hpp"
#include "Component.hpp"
#include <utility>

namespace nen {
class draw_2d_component : public base_component {
public:
  /**
   * @brief コンストラクタ
   *
   * @param owner 親となるアクター
   * @param drawOrder 描画順序
   * @param tex テクスチャ
   */
  draw_2d_component(class base_actor &owner, int drawOrder = 100);
  ~draw_2d_component();

  /**
   * @brief Actorから呼ばれる更新関数
   *
   * @param deltaTime 変位時間
   */
  virtual void Update(float deltaTime) override;

  /**
   * @brief DrawObjectを使用する
   *
   * @param texture 使用するテクスチャ
   * @param scale スケール
   * @param shape 形
   */
  virtual void Create(handle_t texture_handle, float scale = 1.0f,
                      std::string_view shape = "SPRITE");

  /**
   * @brief 生成したDrawObjectを登録する
   *
   */
  void Register();

  int GetDrawOrder() const { return mDrawOrder; }
  int GetTexHeight() const { return mTexHeight; }
  int GetTexWidth() const { return mTexWidth; }

  std::shared_ptr<class draw_object> GetSprite() { return this->sprite; }

  void SetScale(const vector2 &scale) {
    mTexWidth = static_cast<int>(mTexWidth * scale.x);
    mTexHeight = static_cast<int>(mTexHeight * scale.y);
  }

private:
  std::shared_ptr<class draw_object> sprite;
  int mDrawOrder = 0;
  int mTexWidth = 0;
  int mTexHeight = 0;
};
} // namespace nen
