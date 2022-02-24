#pragma once
#include "../Math/Vector2.hpp"
#include "../Utility/handler.hpp"
#include "Component.hpp"
namespace nen {
class draw_3d_component : public base_component {
public:
  /**
   * @brief コンストラクタ
   *
   * @param owner 親となるアクター
   * @param drawOrder 描画順序
   */
  draw_3d_component(class base_actor &owner, int drawOrder = 100);
  /**
   * @brief デストラクタ
   *
   */
  ~draw_3d_component();

  /**
   * @brief Actorから呼ばれる更新関数
   *
   * @param deltaTime 変位時間
   */
  void Update(float deltaTime) override;

  /**
   * @brief DrawObjectを生成する
   *
   * @param texture 使用するテクスチャ
   * @param shape 使用する頂点配列
   */
  void Create(handle_t handle, std::string_view shape = "SPRITE");

  /**
   * @brief 生成したDrawObjectを登録する
   *
   */
  void Register();

  /**
   * @brief Get the Tex Height object
   *
   * @return int
   */
  int GetTexHeight() const { return mTexHeight; }
  /**
   * @brief Get the Tex Width object
   *
   * @return int
   */
  int GetTexWidth() const { return mTexWidth; }
  std::shared_ptr<class draw_object> GetSprite() { return this->sprite; }
  std::shared_ptr<class draw_object> sprite;

private:
  int mTexWidth = 0;
  int mTexHeight = 0;
  base_actor &mOwner;
};
} // namespace nen
